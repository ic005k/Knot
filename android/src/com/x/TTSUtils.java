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
    private static TTSUtils instance;
    private final Context context; // 声明为 final
    private TextToSpeech textToSpeech;
    private AudioManager audioManager;
    private final Handler mainHandler;
    private boolean isInitialized = false;
    private boolean audioFocusRestoreNeeded = false;

    // ========== 新增：分段播放相关变量 ==========
    private Queue<String> playQueue = new ConcurrentLinkedQueue<>(); // 文本片段队列
    private boolean isPlayingSegment = false; // 是否正在播放分段
    private static final int SEGMENT_MAX_LENGTH = 100; // 每段最大字符数（可根据测试调整）

    // ========== 新增：播放完成回调接口 ==========
    public interface OnPlayCompleteListener {
        /**
         * 长文本分段播放完成时触发
         */
        void onPlayComplete();

        /**
         * 播放被停止时触发（可选）
         */
        void onPlayStopped();
    }

    // 播放完成监听器实例
    private OnPlayCompleteListener playCompleteListener;

    // 对外提供设置监听器的方法
    public void setOnPlayCompleteListener(OnPlayCompleteListener listener) {
        this.playCompleteListener = listener;
    }

    // ==========================================

    // 单例模式
    public static synchronized TTSUtils getInstance(Context context) {
        if (instance == null) {
            instance = new TTSUtils(context.getApplicationContext());
        }
        return instance;
    }

    private TTSUtils(Context context) {
        this.context = context;
        this.mainHandler = new Handler(Looper.getMainLooper());
        initAudioManager();
    }

    /**
     * 初始化TTS
     */
    public void initialize(final InitCallback callback) {
        // MIUI权限处理
        if (Build.MANUFACTURER.equalsIgnoreCase("xiaomi")) {
            // requestXiaomiPermissions();
        }

        new Thread(() -> {
            try {
                final CountDownLatch latch = new CountDownLatch(1);
                final boolean[] initSuccess = { false };

                Log.d(TAG, "Initializing TTS with system default engine");
                textToSpeech = new TextToSpeech(context, status -> {
                    if (status == TextToSpeech.SUCCESS) {
                        Log.d(TAG, "TTS initialized successfully");

                        // 设置语言
                        int result = textToSpeech.setLanguage(
                            Locale.getDefault()
                        );

                        if (
                            result == TextToSpeech.LANG_MISSING_DATA ||
                            result == TextToSpeech.LANG_NOT_SUPPORTED
                        ) {
                            // 尝试英语作为备选
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

                        // ========== 修改：重写UtteranceProgressListener，添加分段续播逻辑 ==========
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
                                    // 播放完成后，自动播放下一段
                                    playNextSegment();
                                    // 只有队列空时才释放音频焦点（避免分段播放时频繁释放）
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
                        // ==============================================================

                        // 使用默认音调和语速
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

                // 等待初始化完成
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

    // ========== 新增：文本分段方法（按标点拆分，避免语义断裂） ==========
    private List<String> splitLongText(String longText) {
        List<String> segments = new ArrayList<>();
        if (longText == null || longText.isEmpty()) {
            return segments;
        }

        // 优化：兼容中英双语标点（。！？；.?!;），按句子拆分
        String[] sentences = longText.split("(?<=[。！？；.?!;])");
        for (String sentence : sentences) {
            sentence = sentence.trim();
            if (sentence.isEmpty()) {
                continue;
            }
            // 若单句仍超长，按SEGMENT_MAX_LENGTH拆分
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

    // ========== 新增：播放下一段文本 ==========
    private void playNextSegment() {
        if (playQueue.isEmpty()) {
            Log.d(TAG, "分段播放队列已空，结束播放");
            // ========== 新增：触发播放完成回调 ==========
            if (playCompleteListener != null) {
                mainHandler.post(() -> playCompleteListener.onPlayComplete());
            }
            // ==========================================
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

    // ==========================================

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
            // 备用方案
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

    // ========== 修改：speak方法，新增长文本分段逻辑 ==========
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

        // 设置语言
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

        // ========== 核心修改：判断文本长度，长文本则分段 ==========
        if (text.length() > SEGMENT_MAX_LENGTH) {
            Log.d(TAG, "检测到长文本，自动分段（长度：" + text.length() + "）");
            // 清空原有队列，停止当前播放
            stopSegmentPlay();
            // 拆分文本并加入队列
            List<String> segments = splitLongText(text);
            playQueue.addAll(segments);
            Log.d(TAG, "拆分后得到 " + segments.size() + " 个片段");
            // 请求音频焦点（仅第一次请求）
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
                // 启动第一段播放
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
        // ======================================================

        // 短文本逻辑（保持原有不变）
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

    // ========== 新增：停止分段播放并清空队列 ==========
    private void stopSegmentPlay() {
        if (isPlayingSegment) {
            textToSpeech.stop();
            isPlayingSegment = false;
        }
        playQueue.clear();
    }

    // ================================================

    public void stop() {
        stopSegmentPlay(); // 新增：停止分段播放
        if (textToSpeech != null && textToSpeech.isSpeaking()) {
            textToSpeech.stop();
        }
        releaseAudioFocus();
    }

    public void shutdown() {
        stopSegmentPlay(); // 新增：停止分段播放
        if (textToSpeech != null) {
            textToSpeech.stop();
            textToSpeech.shutdown();
            textToSpeech = null;
            isInitialized = false;
        }
        releaseAudioFocus();
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
                    // 重要：在这里标记焦点丢失状态
                    audioFocusRestoreNeeded = true;
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    Log.w(TAG, "Temporary loss with ducking request");
                    // 对于TTS，我们选择停止而不是降低音量
                    stop();
                    // 重要：在这里标记焦点丢失状态
                    audioFocusRestoreNeeded = true;
                    break;
                case AudioManager.AUDIOFOCUS_GAIN:
                    Log.d(TAG, "Audio focus regained");
                    // 重置状态，准备下次播放
                    audioFocusRestoreNeeded = false;
                    break;
            }
        };
}
