package com.x;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import android.util.Log;
import android.widget.Toast;
import java.lang.ref.WeakReference;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.ReentrantLock;

/**
 * 兼容旧版本的 TTSUtils
 * 保留了原始接口，但修复了关键问题
 */
public class TTSUtils implements AudioManager.OnAudioFocusChangeListener {

    private static final String TAG = "TTSUtils";

    // ========== 与旧版本兼容的接口 ==========
    public interface InitCallback {
        void onSuccess();
        void onError(String error);
    }

    public interface OnPlayCompleteListener {
        void onPlayComplete();
        void onPlayStopped();
    }

    // ========== 新的监听器接口（可选） ==========
    public interface EnhancedInitListener {
        void onSuccess();
        void onError(String error);
    }

    public interface EnhancedPlayListener {
        void onPlayStart();
        void onPlayComplete();
        void onPlayStopped();
        void onPlayError(String error);
    }

    // ========== 内部类避免内存泄漏 ==========
    private static class SafeAudioFocusListener
        implements AudioManager.OnAudioFocusChangeListener {

        private final WeakReference<TTSUtils> utilsRef;

        SafeAudioFocusListener(TTSUtils utils) {
            this.utilsRef = new WeakReference<>(utils);
        }

        @Override
        public void onAudioFocusChange(int focusChange) {
            TTSUtils utils = utilsRef.get();
            if (utils != null) {
                utils.handleAudioFocusChange(focusChange);
            }
        }
    }

    // ========== 成员变量 ==========
    private final Context context;
    private final Handler mainHandler;
    private final ReentrantLock lock = new ReentrantLock();
    private final Object initLock = new Object();

    private TextToSpeech textToSpeech;
    private AudioManager audioManager;
    private final SafeAudioFocusListener audioFocusListener;

    // 原子状态变量
    private final AtomicBoolean isInitialized = new AtomicBoolean(false);
    private final AtomicBoolean isInitializing = new AtomicBoolean(false);
    private final AtomicBoolean isPlayingSegment = new AtomicBoolean(false);
    private final AtomicBoolean audioFocusRestoreNeeded = new AtomicBoolean(
        false
    );
    private final AtomicBoolean isReleased = new AtomicBoolean(false);

    // 播放队列
    private final Deque<String> playQueue = new ArrayDeque<>();

    // 监听器
    private InitCallback initCallback;
    private OnPlayCompleteListener playCompleteListener;
    private EnhancedInitListener enhancedInitListener;
    private EnhancedPlayListener enhancedPlayListener;

    // 配置
    private static final int SEGMENT_MAX_LENGTH = 100;

    // ========== 构造方法 ==========
    public TTSUtils(Context context) {
        this.context = context.getApplicationContext();
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.audioManager = (AudioManager) context.getSystemService(
            Context.AUDIO_SERVICE
        );
        this.audioFocusListener = new SafeAudioFocusListener(this);
    }

    // ========== 公共API（与旧版本兼容） ==========

    /**
     * 初始化TTS - 与旧版本兼容
     */
    public void initialize(final InitCallback callback) {
        if (isReleased.get()) {
            notifyInitError(callback, "TTSUtils has been released");
            return;
        }

        if (isInitialized.get()) {
            notifyInitSuccess(callback);
            return;
        }

        synchronized (initLock) {
            if (isInitializing.get()) {
                // 已经在初始化，合并回调
                mergeInitCallbacks(callback);
                return;
            }
            isInitializing.set(true);
            this.initCallback = callback;
        }

        new Thread(
            () -> {
                try {
                    final CountDownLatch latch = new CountDownLatch(1);
                    final boolean[] initSuccess = { false };

                    mainHandler.post(() -> {
                        if (isReleased.get()) {
                            latch.countDown();
                            return;
                        }

                        Log.d(TAG, "Initializing TTS");
                        textToSpeech = new TextToSpeech(context, status -> {
                            if (status == TextToSpeech.SUCCESS) {
                                setupTtsEngine();
                                isInitialized.set(true);
                                isInitializing.set(false);
                                notifyInitSuccess(initCallback);
                            } else {
                                String errorMsg = getInitErrorMsg(status);
                                isInitializing.set(false);
                                notifyInitError(initCallback, errorMsg);
                                cleanupFailedInit();
                            }
                            latch.countDown();
                        });
                    });

                    if (!latch.await(5, TimeUnit.SECONDS)) {
                        mainHandler.post(() -> {
                            Log.w(TAG, "TTS initialization timeout");
                            isInitializing.set(false);
                            notifyInitError(
                                initCallback,
                                "TTS initialization timed out"
                            );
                            if (textToSpeech != null) {
                                try {
                                    textToSpeech.shutdown();
                                } catch (Exception e) {
                                    Log.e(
                                        TAG,
                                        "Error shutting down TTS after timeout",
                                        e
                                    );
                                }
                                textToSpeech = null;
                            }
                        });
                    }
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    isInitializing.set(false);
                    notifyInitError(initCallback, "Initialization interrupted");
                } catch (Exception e) {
                    Log.e(TAG, "TTS initialization exception", e);
                    isInitializing.set(false);
                    notifyInitError(
                        initCallback,
                        "Initialization exception: " + e.getMessage()
                    );
                }
            },
            "TTS-Init-Thread"
        )
            .start();
    }

    /**
     * 设置播放完成监听器 - 与旧版本兼容
     */
    public void setOnPlayCompleteListener(OnPlayCompleteListener listener) {
        this.playCompleteListener = listener;
    }

    /**
     * 设置增强版初始化监听器
     */
    public void setOnInitListener(EnhancedInitListener listener) {
        this.enhancedInitListener = listener;
    }

    /**
     * 设置增强版播放监听器
     */
    public void setOnPlayListener(EnhancedPlayListener listener) {
        this.enhancedPlayListener = listener;
    }

    /**
     * 播放文本 - 与旧版本兼容
     */
    public void speak(String text) {
        speak(text, Locale.getDefault());
    }

    public void speak(String text, Locale locale) {
        if (isReleased.get()) {
            Log.w(TAG, "Cannot speak: TTSUtils released");
            Toast.makeText(
                context,
                "TTS not available",
                Toast.LENGTH_SHORT
            ).show();
            return;
        }

        if (!isInitialized.get()) {
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

        // 检查语言支持
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

        // 处理长文本分段
        if (text.length() > SEGMENT_MAX_LENGTH) {
            Log.d(TAG, "检测到长文本，自动分段（长度：" + text.length() + "）");
            stopSegmentPlay();
            List<String> segments = splitLongText(text);
            playQueue.clear();
            playQueue.addAll(segments);
            Log.d(TAG, "拆分后得到 " + segments.size() + " 个片段");

            if (audioFocusRestoreNeeded.get()) {
                Log.d(TAG, "Restoring lost audio focus");
                releaseAudioFocus();
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

        // 短文本直接播放
        if (audioFocusRestoreNeeded.get()) {
            Log.d(TAG, "Restoring lost audio focus");
            releaseAudioFocus();
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
                textToSpeech.speak(
                    text,
                    TextToSpeech.QUEUE_ADD,
                    null,
                    utteranceId
                );
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

    /**
     * 停止播放 - 与旧版本兼容
     */
    public void stop() {
        lock.lock();
        try {
            stopSegmentPlay();
            if (textToSpeech != null && textToSpeech.isSpeaking()) {
                textToSpeech.stop();
            }
            releaseAudioFocus();

            if (playCompleteListener != null) {
                mainHandler.post(() -> playCompleteListener.onPlayStopped());
            }
            if (enhancedPlayListener != null) {
                mainHandler.post(() -> enhancedPlayListener.onPlayStopped());
            }
        } finally {
            lock.unlock();
        }
    }

    /**
     * 关闭TTS - 与旧版本兼容（保留shutdown方法名）
     */
    public void shutdown() {
        if (isReleased.getAndSet(true)) {
            return;
        }

        lock.lock();
        try {
            stopSegmentPlay();
            if (textToSpeech != null) {
                try {
                    textToSpeech.setOnUtteranceProgressListener(null);
                    textToSpeech.stop();
                    textToSpeech.shutdown();
                } catch (Exception e) {
                    Log.e(TAG, "Error shutting down TTS", e);
                }
                textToSpeech = null;
            }

            releaseAudioFocus();
            if (audioManager != null) {
                audioManager.abandonAudioFocus(audioFocusListener);
            }

            mainHandler.removeCallbacksAndMessages(null);
            playQueue.clear();
            isInitialized.set(false);
            isInitializing.set(false);
            isPlayingSegment.set(false);

            Log.d(TAG, "TTS instance completely released");
        } finally {
            lock.unlock();
        }
    }

    /**
     * 新增：释放资源（同shutdown，但更语义化）
     */
    public void release() {
        shutdown();
    }

    // ========== 私有辅助方法 ==========
    private void setupTtsEngine() {
        if (textToSpeech == null || isReleased.get()) {
            return;
        }

        try {
            // 设置默认参数
            textToSpeech.setPitch(1.0f);
            textToSpeech.setSpeechRate(1.0f);

            // 设置语言
            int result = textToSpeech.setLanguage(Locale.getDefault());
            if (
                result == TextToSpeech.LANG_MISSING_DATA ||
                result == TextToSpeech.LANG_NOT_SUPPORTED
            ) {
                result = textToSpeech.setLanguage(Locale.ENGLISH);
                if (
                    result == TextToSpeech.LANG_MISSING_DATA ||
                    result == TextToSpeech.LANG_NOT_SUPPORTED
                ) {
                    Log.w(TAG, "Default and English languages not supported");
                }
            }

            // 设置监听器
            textToSpeech.setOnUtteranceProgressListener(
                new UtteranceProgressListener() {
                    @Override
                    public void onStart(String utteranceId) {
                        Log.d(TAG, "Speech started: " + utteranceId);
                        isPlayingSegment.set(true);

                        if (enhancedPlayListener != null) {
                            mainHandler.post(() ->
                                enhancedPlayListener.onPlayStart()
                            );
                        }
                    }

                    @Override
                    public void onDone(String utteranceId) {
                        Log.d(TAG, "Speech completed: " + utteranceId);
                        isPlayingSegment.set(false);
                        playNextSegment();

                        if (playQueue.isEmpty()) {
                            releaseAudioFocus();
                            if (playCompleteListener != null) {
                                mainHandler.post(() ->
                                    playCompleteListener.onPlayComplete()
                                );
                            }
                            if (enhancedPlayListener != null) {
                                mainHandler.post(() ->
                                    enhancedPlayListener.onPlayComplete()
                                );
                            }
                        }
                    }

                    @Override
                    public void onError(String utteranceId) {
                        Log.e(TAG, "Speech error: " + utteranceId);
                        isPlayingSegment.set(false);
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

                            if (enhancedPlayListener != null) {
                                mainHandler.post(() ->
                                    enhancedPlayListener.onPlayError(
                                        "Speech error: " + utteranceId
                                    )
                                );
                            }
                        }
                    }
                }
            );

            Log.d(TAG, "TTS engine setup completed");
        } catch (Exception e) {
            Log.e(TAG, "Error setting up TTS engine", e);
        }
    }

    private void stopSegmentPlay() {
        lock.lock();
        try {
            if (isPlayingSegment.get()) {
                if (textToSpeech != null) {
                    textToSpeech.stop();
                }
                isPlayingSegment.set(false);
            }
            playQueue.clear();
        } finally {
            lock.unlock();
        }
    }

    private void playNextSegment() {
        if (isReleased.get() || textToSpeech == null) {
            return;
        }

        lock.lock();
        try {
            if (playQueue.isEmpty()) {
                Log.d(TAG, "分段播放队列已空，结束播放");
                return;
            }

            String nextSegment = playQueue.poll();
            if (nextSegment == null || nextSegment.isEmpty()) {
                // 跳过空片段，继续下一个
                mainHandler.post(this::playNextSegment);
                return;
            }

            Log.d(
                TAG,
                "播放下一段：" +
                    nextSegment.substring(
                        0,
                        Math.min(20, nextSegment.length())
                    ) +
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
                textToSpeech.speak(
                    nextSegment,
                    TextToSpeech.QUEUE_FLUSH,
                    null,
                    utteranceId
                );
            }
        } finally {
            lock.unlock();
        }
    }

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

    private int requestAudioFocus() {
        if (audioManager == null || isReleased.get()) {
            return AudioManager.AUDIOFOCUS_REQUEST_FAILED;
        }

        try {
            return audioManager.requestAudioFocus(
                audioFocusListener,
                AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK
            );
        } catch (Exception e) {
            Log.e(TAG, "Error requesting audio focus", e);
            return AudioManager.AUDIOFOCUS_REQUEST_FAILED;
        }
    }

    private void releaseAudioFocus() {
        if (audioManager != null && !isReleased.get()) {
            try {
                audioManager.abandonAudioFocus(audioFocusListener);
                audioFocusRestoreNeeded.set(false);
                Log.d(TAG, "Audio focus released");
            } catch (Exception e) {
                Log.e(TAG, "Error releasing audio focus", e);
            }
        }
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
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
                audioFocusRestoreNeeded.set(true);
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                Log.w(TAG, "Temporary loss with ducking request");
                stop();
                audioFocusRestoreNeeded.set(true);
                break;
            case AudioManager.AUDIOFOCUS_GAIN:
                Log.d(TAG, "Audio focus regained");
                audioFocusRestoreNeeded.set(false);
                break;
        }
    }

    private void handleAudioFocusChange(int focusChange) {
        mainHandler.post(() -> onAudioFocusChange(focusChange));
    }

    private void cleanupFailedInit() {
        if (textToSpeech != null) {
            try {
                textToSpeech.shutdown();
            } catch (Exception e) {
                Log.e(TAG, "Error shutting down TTS after failed init", e);
            }
            textToSpeech = null;
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

    private void notifyInitSuccess(InitCallback callback) {
        if (callback != null) {
            mainHandler.post(callback::onSuccess);
        }
        if (enhancedInitListener != null) {
            mainHandler.post(enhancedInitListener::onSuccess);
        }
    }

    private void notifyInitError(InitCallback callback, String error) {
        if (callback != null) {
            mainHandler.post(() -> callback.onError(error));
        }
        if (enhancedInitListener != null) {
            mainHandler.post(() -> enhancedInitListener.onError(error));
        }
    }

    private void mergeInitCallbacks(InitCallback newCallback) {
        if (newCallback == null) {
            return;
        }

        final InitCallback oldCallback = this.initCallback;
        this.initCallback = new InitCallback() {
            @Override
            public void onSuccess() {
                if (oldCallback != null) oldCallback.onSuccess();
                newCallback.onSuccess();
            }

            @Override
            public void onError(String error) {
                if (oldCallback != null) oldCallback.onError(error);
                newCallback.onError(error);
            }
        };
    }

    // ========== 新增辅助方法 ==========

    /**
     * 检查TTS是否已初始化
     */
    public boolean isInitialized() {
        return isInitialized.get() && !isReleased.get();
    }

    /**
     * 检查是否正在播放
     */
    public boolean isSpeaking() {
        return (
            (textToSpeech != null && textToSpeech.isSpeaking()) ||
            isPlayingSegment.get()
        );
    }

    /**
     * 设置语速
     */
    public void setSpeechRate(float rate) {
        if (textToSpeech != null && !isReleased.get()) {
            textToSpeech.setSpeechRate(rate);
        }
    }

    /**
     * 设置音调
     */
    public void setPitch(float pitch) {
        if (textToSpeech != null && !isReleased.get()) {
            textToSpeech.setPitch(pitch);
        }
    }

    /**
     * 设置语言
     */
    public boolean setLanguage(Locale locale) {
        if (textToSpeech != null && !isReleased.get()) {
            int result = textToSpeech.setLanguage(locale);
            return (
                result != TextToSpeech.LANG_MISSING_DATA &&
                result != TextToSpeech.LANG_NOT_SUPPORTED
            );
        }
        return false;
    }
}
