package com.x;

import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import android.util.Log;
import android.widget.Toast;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class TTSUtils {

    private static final String TAG = "TTSUtils";
    // 1. 移除单例相关变量
    // private static TTSUtils instance;
    private final Context context; // 声明为 final
    private TextToSpeech textToSpeech;
    private AudioManager audioManager;
    private final Handler mainHandler;
    private boolean isInitialized = false;
    private boolean audioFocusRestoreNeeded = false;

    // ========== 保留：分段播放相关变量（实例级，非静态） ==========
    private Queue<String> playQueue = new ConcurrentLinkedQueue<>(); // 文本片段队列
    private boolean isPlayingSegment = false; // 是否正在播放分段
    private static final int SEGMENT_MAX_LENGTH = 100; // 每段最大字符数（可根据测试调整）

    // ========== 保留：播放完成回调接口 ==========
    public interface OnPlayCompleteListener {
        void onPlayComplete();
        void onPlayStopped();
    }

    private OnPlayCompleteListener playCompleteListener;

    public void setOnPlayCompleteListener(OnPlayCompleteListener listener) {
        this.playCompleteListener = listener;
    }

    // ==========================================

    // 2. 移除单例getInstance方法，改为公共构造方法
    public TTSUtils(Context context) {
        this.context = context.getApplicationContext(); // 仍用Application Context防泄漏
        this.mainHandler = new Handler(Looper.getMainLooper());
        initAudioManager();
    }

    /**
     * 保留：初始化TTS（逻辑不变，仅移除单例依赖）
     */
    public void initialize(final InitCallback callback) {
        // MIUI权限处理（保留）
        if (Build.MANUFACTURER.equalsIgnoreCase("xiaomi")) {
            // requestXiaomiPermissions();
        }

        new Thread(() -> {
            try {
                final CountDownLatch latch = new CountDownLatch(1);
                final boolean[] initSuccess = { false };

                Log.d(TAG, "Initializing TTS with system default engine");
                // 每次初始化都新建TextToSpeech实例（核心）
                textToSpeech = new TextToSpeech(context, status -> {
                    if (status == TextToSpeech.SUCCESS) {
                        Log.d(TAG, "TTS initialized successfully");

                        // 设置语言（保留）
                        int result = textToSpeech.setLanguage(
                            Locale.getDefault()
                        );

                        if (
                            result == TextToSpeech.LANG_MISSING_DATA ||
                            result == TextToSpeech.LANG_NOT_SUPPORTED
                        ) {
                            result = textToSpeech.setLanguage(Locale.ENGLISH);
                            if (
                                result == TextToSpeech.LANG_MISSING_DATA ||
                                result == TextToSpeech.LANG_NOT_SUPPORTED
                            ) {
                                Log.w(
                                    TAG,
                                    "Default and English languages not supported"
                                );
                                mainHandler.post(() -> {
                                    if (callback != null) callback.onError(
                                        "Language not supported"
                                    );
                                });
                                return;
                            }
                        }

                        // 保留：分段播放的监听器逻辑
                        textToSpeech.setOnUtteranceProgressListener(
                            new UtteranceProgressListener() {
                                @Override
                                public void onStart(String utteranceId) {
                                    Log.d(
                                        TAG,
                                        "Speech started: " + utteranceId
                                    );
                                    isPlayingSegment = true;
                                }

                                @Override
                                public void onDone(String utteranceId) {
                                    Log.d(
                                        TAG,
                                        "Speech completed: " + utteranceId
                                    );
                                    isPlayingSegment = false;
                                    playNextSegment();
                                    if (playQueue.isEmpty()) {
                                        releaseAudioFocus();
                                    }
                                }

                                @Override
                                public void onError(String utteranceId) {
                                    Log.e(TAG, "Speech error: " + utteranceId);
                                    isPlayingSegment = false;
                                    playNextSegment();
                                    if (playQueue.isEmpty()) {
                                        releaseAudioFocus();
                                        mainHandler.post(() ->
                                            Toast.makeText(
                                                context,
                                                "Speech error",
                                                Toast.LENGTH_SHORT
                                            ).show()
                                        );
                                    }
                                }
                            }
                        );

                        // 保留：默认音调和语速
                        textToSpeech.setPitch(1.0f);
                        textToSpeech.setSpeechRate(1.0f);

                        initSuccess[0] = true;
                        isInitialized = true;
                        mainHandler.post(() -> {
                            if (callback != null) callback.onSuccess();
                        });
                    } else {
                        String errorMsg = getInitErrorMsg(status);
                        Log.e(TAG, "TTS init failed: " + errorMsg);
                        mainHandler.post(() -> {
                            if (callback != null) callback.onError(errorMsg);
                        });
                    }
                    latch.countDown();
                });

                // 等待初始化完成（保留）
                latch.await(5, TimeUnit.SECONDS);

                if (!initSuccess[0] && callback != null) {
                    mainHandler.post(() ->
                        callback.onError("TTS initialization timed out")
                    );
                }
            } catch (Exception e) {
                Log.e(TAG, "TTS initialization exception", e);
                if (callback != null) {
                    mainHandler.post(() ->
                        callback.onError(
                            "Initialization exception: " + e.getMessage()
                        )
                    );
                }
            }
        })
            .start();
    }

    // ========== 保留：所有原有核心逻辑（分段、播放、音频焦点等） ==========
    private List<String> splitLongText(String longText) {
        List<String> segments = new ArrayList<>();
        if (longText == null || longText.isEmpty()) {
            return segments;
        }

        String[] sentences = longText.split("(?<=[。！？；.?!;])");
        for (String sentence : sentences) {
            sentence = sentence.trim();
            if (sentence.isEmpty()) {
                continue;
            }
            if (sentence.length() > SEGMENT_MAX_LENGTH) {
                for (
                    int i = 0;
                    i < sentence.length();
                    i += SEGMENT_MAX_LENGTH
                ) {
                    int end = Math.min(
                        i + SEGMENT_MAX_LENGTH,
                        sentence.length()
                    );
                    segments.add(sentence.substring(i, end));
                }
            } else {
                segments.add(sentence);
            }
        }
        return segments;
    }

    private void playNextSegment() {
        if (playQueue.isEmpty()) {
            Log.d(TAG, "分段播放队列已空，结束播放");
            if (playCompleteListener != null) {
                mainHandler.post(() -> playCompleteListener.onPlayComplete());
            }
            return;
        }

        String nextSegment = playQueue.poll();
        if (nextSegment == null || nextSegment.isEmpty()) {
            playNextSegment();
            return;
        }

        Log.d(
            TAG,
            "播放下一段：" +
                nextSegment.substring(0, Math.min(20, nextSegment.length())) +
                "..."
        );
        String utteranceId = "tts_segment_" + System.currentTimeMillis();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Bundle params = new Bundle();
            textToSpeech.speak(
                nextSegment,
                TextToSpeech.QUEUE_FLUSH,
                params,
                utteranceId
            );
        } else {
            textToSpeech.speak(nextSegment, TextToSpeech.QUEUE_FLUSH, null);
        }
    }

    private void requestXiaomiPermissions() {
        try {
            Intent intent = new Intent("miui.intent.action.APP_PERM_EDITOR");
            intent.setClassName(
                "com.miui.securitycenter",
                "com.miui.permcenter.permissions.PermissionsEditorActivity"
            );
            intent.putExtra("extra_pkgname", context.getPackageName());
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            Intent intent = new Intent(
                Settings.ACTION_APPLICATION_DETAILS_SETTINGS
            );
            intent.setData(Uri.parse("package:" + context.getPackageName()));
            context.startActivity(intent);
        }
    }

    private String getInitErrorMsg(int status) {
        switch (status) {
            case TextToSpeech.ERROR:
                return "TTS engine error";
            case TextToSpeech.ERROR_INVALID_REQUEST:
                return "Invalid request";
            case TextToSpeech.ERROR_NETWORK:
                return "Network error";
            case TextToSpeech.ERROR_NETWORK_TIMEOUT:
                return "Network timeout";
            case TextToSpeech.ERROR_NOT_INSTALLED_YET:
                return "TTS engine not installed";
            case TextToSpeech.ERROR_OUTPUT:
                return "Output error";
            case TextToSpeech.ERROR_SERVICE:
                return "TTS service error";
            case TextToSpeech.ERROR_SYNTHESIS:
                return "Synthesis error";
            default:
                return "Unknown error (" + status + ")";
        }
    }

    private void initAudioManager() {
        audioManager = (AudioManager) context.getSystemService(
            Context.AUDIO_SERVICE
        );
    }

    public void speak(String text) {
        speak(text, Locale.getDefault());
    }

    public void speak(String text, Locale locale) {
        if (!isInitialized) {
            Log.w(TAG, "Trying to speak before initialization");
            Toast.makeText(
                context,
                "TTS not initialized",
                Toast.LENGTH_SHORT
            ).show();
            initialize(
                new InitCallback() {
                    @Override
                    public void onSuccess() {
                        speak(text, locale);
                    }

                    @Override
                    public void onError(String error) {
                        Toast.makeText(
                            context,
                            "Initialization failed: " + error,
                            Toast.LENGTH_SHORT
                        ).show();
                    }
                }
            );
            return;
        }

        int result = textToSpeech.setLanguage(locale);
        if (
            result == TextToSpeech.LANG_MISSING_DATA ||
            result == TextToSpeech.LANG_NOT_SUPPORTED
        ) {
            result = textToSpeech.setLanguage(Locale.ENGLISH);
            if (
                result == TextToSpeech.LANG_MISSING_DATA ||
                result == TextToSpeech.LANG_NOT_SUPPORTED
            ) {
                Toast.makeText(
                    context,
                    "Language not supported",
                    Toast.LENGTH_SHORT
                ).show();
                return;
            }
        }

        if (text.length() > SEGMENT_MAX_LENGTH) {
            Log.d(TAG, "检测到长文本，自动分段（长度：" + text.length() + "）");
            stopSegmentPlay();
            List<String> segments = splitLongText(text);
            playQueue.addAll(segments);
            Log.d(TAG, "拆分后得到 " + segments.size() + " 个片段");
            if (audioFocusRestoreNeeded) {
                Log.d(TAG, "Restoring lost audio focus");
                releaseAudioFocus();
                if (
                    requestAudioFocus() !=
                    AudioManager.AUDIOFOCUS_REQUEST_GRANTED
                ) {
                    Log.w(TAG, "Failed to restore audio focus");
                    return;
                }
            }
            int focusResult = requestAudioFocus();
            if (focusResult == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                playNextSegment();
            } else {
                Log.w(TAG, "Audio focus denied");
                Toast.makeText(
                    context,
                    "Audio focus denied",
                    Toast.LENGTH_SHORT
                ).show();
            }
            return;
        }

        if (audioFocusRestoreNeeded) {
            Log.d(TAG, "Restoring lost audio focus");
            releaseAudioFocus();
            if (
                requestAudioFocus() != AudioManager.AUDIOFOCUS_REQUEST_GRANTED
            ) {
                Log.w(TAG, "Failed to restore audio focus");
                return;
            }
        }

        int focusResult = requestAudioFocus();
        if (focusResult == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            Log.d(TAG, "Speaking: " + text);
            String utteranceId = "tts_" + System.currentTimeMillis();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                Bundle params = new Bundle();
                textToSpeech.speak(
                    text,
                    TextToSpeech.QUEUE_ADD,
                    params,
                    utteranceId
                );
            } else {
                textToSpeech.speak(text, TextToSpeech.QUEUE_ADD, null);
            }
        } else {
            Log.w(TAG, "Audio focus denied");
            Toast.makeText(
                context,
                "Audio focus denied",
                Toast.LENGTH_SHORT
            ).show();
        }
    }

    private void stopSegmentPlay() {
        if (isPlayingSegment) {
            textToSpeech.stop();
            isPlayingSegment = false;
        }
        playQueue.clear();
    }

    public void stop() {
        stopSegmentPlay();
        if (textToSpeech != null && textToSpeech.isSpeaking()) {
            textToSpeech.stop();
        }
        releaseAudioFocus();
    }

    // 增强：shutdown时主动清空队列+标记未初始化
    public void shutdown() {
        stopSegmentPlay();
        if (textToSpeech != null) {
            textToSpeech.stop();
            textToSpeech.shutdown();
            textToSpeech = null;
        }
        playQueue.clear(); // 清空队列
        isInitialized = false; // 重置状态
        releaseAudioFocus();
        Log.d(TAG, "TTS实例已完全释放");
    }

    public interface InitCallback {
        void onSuccess();
        void onError(String error);
    }

    private int requestAudioFocus() {
        if (audioManager == null) {
            return AudioManager.AUDIOFOCUS_REQUEST_FAILED;
        }

        return audioManager.requestAudioFocus(
            audioFocusListener,
            AudioManager.STREAM_MUSIC,
            AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK
        );
    }

    private void releaseAudioFocus() {
        if (audioManager != null) {
            audioManager.abandonAudioFocus(audioFocusListener);
            audioFocusRestoreNeeded = false;
            Log.d(TAG, "Audio focus released");
        }
    }

    private AudioManager.OnAudioFocusChangeListener audioFocusListener =
        focusChange -> {
            Log.d(TAG, "Audio focus change: " + focusChange);

            switch (focusChange) {
                case AudioManager.AUDIOFOCUS_LOSS:
                    Log.w(TAG, "Permanent audio focus loss");
                    stop();
                    releaseAudioFocus();
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                    Log.w(TAG, "Temporary audio focus loss");
                    stop();
                    audioFocusRestoreNeeded = true;
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    Log.w(TAG, "Temporary loss with ducking request");
                    stop();
                    audioFocusRestoreNeeded = true;
                    break;
                case AudioManager.AUDIOFOCUS_GAIN:
                    Log.d(TAG, "Audio focus regained");
                    audioFocusRestoreNeeded = false;
                    break;
            }
        };
}
