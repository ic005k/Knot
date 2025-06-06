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
import java.util.Locale;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import android.provider.Settings;
import android.net.Uri;
import android.content.Intent;

public class TTSUtils {
    private static final String TAG = "TTSUtils";
    private static TTSUtils instance;
    private final Context context; // 声明为 final
    private TextToSpeech textToSpeech;
    private AudioManager audioManager;
    private final Handler mainHandler;
    private boolean isInitialized = false;

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
                        int result = textToSpeech.setLanguage(Locale.getDefault());

                        if (result == TextToSpeech.LANG_MISSING_DATA ||
                                result == TextToSpeech.LANG_NOT_SUPPORTED) {
                            // 尝试英语作为备选
                            result = textToSpeech.setLanguage(Locale.ENGLISH);

                            if (result == TextToSpeech.LANG_MISSING_DATA ||
                                    result == TextToSpeech.LANG_NOT_SUPPORTED) {
                                Log.w(TAG, "Default and English languages not supported");
                                mainHandler.post(() -> {
                                    if (callback != null)
                                        callback.onError("Language not supported");
                                });
                                return;
                            }
                        }

                        // 设置合成回调
                        textToSpeech.setOnUtteranceProgressListener(new UtteranceProgressListener() {
                            @Override
                            public void onStart(String utteranceId) {
                                Log.d(TAG, "Speech started: " + utteranceId);
                            }

                            @Override
                            public void onDone(String utteranceId) {
                                Log.d(TAG, "Speech completed: " + utteranceId);
                                releaseAudioFocus();
                            }

                            @Override
                            public void onError(String utteranceId) {
                                Log.e(TAG, "Speech error: " + utteranceId);
                                releaseAudioFocus();
                                mainHandler
                                        .post(() -> Toast.makeText(context, "Speech error", Toast.LENGTH_SHORT).show());
                            }
                        });

                        // 使用默认音调和语速
                        textToSpeech.setPitch(1.0f);
                        textToSpeech.setSpeechRate(1.0f);

                        initSuccess[0] = true;
                        isInitialized = true;
                        mainHandler.post(() -> {
                            if (callback != null)
                                callback.onSuccess();
                        });
                    } else {
                        String errorMsg = getInitErrorMsg(status);
                        Log.e(TAG, "TTS init failed: " + errorMsg);
                        mainHandler.post(() -> {
                            if (callback != null)
                                callback.onError(errorMsg);
                        });
                    }
                    latch.countDown();
                });

                // 等待初始化完成
                latch.await(5, TimeUnit.SECONDS);

                if (!initSuccess[0] && callback != null) {
                    mainHandler.post(() -> callback.onError("TTS initialization timed out"));
                }

            } catch (Exception e) {
                Log.e(TAG, "TTS initialization exception", e);
                if (callback != null) {
                    mainHandler.post(() -> callback.onError("Initialization exception: " + e.getMessage()));
                }
            }
        }).start();

    }

    private void requestXiaomiPermissions() {
        try {
            Intent intent = new Intent("miui.intent.action.APP_PERM_EDITOR");
            intent.setClassName("com.miui.securitycenter",
                    "com.miui.permcenter.permissions.PermissionsEditorActivity");
            intent.putExtra("extra_pkgname", context.getPackageName());
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            // 备用方案
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
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
        audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
    }

    public void speak(String text) {
        speak(text, Locale.getDefault());
    }

    public void speak(String text, Locale locale) {
        if (!isInitialized) {
            Log.w(TAG, "Trying to speak before initialization");
            Toast.makeText(context, "TTS not initialized", Toast.LENGTH_SHORT).show();
            initialize(new InitCallback() {
                @Override
                public void onSuccess() {
                    speak(text, locale);
                }

                @Override
                public void onError(String error) {
                    Toast.makeText(context, "Initialization failed: " + error, Toast.LENGTH_SHORT).show();
                }
            });
            return;
        }

        // 设置语言
        int result = textToSpeech.setLanguage(locale);
        if (result == TextToSpeech.LANG_MISSING_DATA ||
                result == TextToSpeech.LANG_NOT_SUPPORTED) {
            // 尝试英语作为备选
            result = textToSpeech.setLanguage(Locale.ENGLISH);
            if (result == TextToSpeech.LANG_MISSING_DATA ||
                    result == TextToSpeech.LANG_NOT_SUPPORTED) {
                Toast.makeText(context, "Language not supported", Toast.LENGTH_SHORT).show();
                return;
            }
        }

        // 请求音频焦点
        int focusResult = audioManager.requestAudioFocus(
                focusChange -> {
                    switch (focusChange) {
                        case AudioManager.AUDIOFOCUS_LOSS:
                            stop();
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                            if (textToSpeech != null && textToSpeech.isSpeaking()) {
                                textToSpeech.stop();
                            }
                            break;
                    }
                },
                AudioManager.STREAM_MUSIC,
                AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK);

        if (focusResult == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            Log.d(TAG, "Speaking: " + text);

            String utteranceId = "tts_" + System.currentTimeMillis();

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                Bundle params = new Bundle();
                textToSpeech.speak(text, TextToSpeech.QUEUE_ADD, params, utteranceId);
            } else {
                textToSpeech.speak(text, TextToSpeech.QUEUE_ADD, null);
            }
        } else {
            Log.w(TAG, "Audio focus denied");
            Toast.makeText(context, "Audio focus denied", Toast.LENGTH_SHORT).show();
        }
    }

    public void stop() {
        if (textToSpeech != null && textToSpeech.isSpeaking()) {
            textToSpeech.stop();
        }
        releaseAudioFocus();
    }

    public void shutdown() {
        if (textToSpeech != null) {
            textToSpeech.stop();
            textToSpeech.shutdown();
            textToSpeech = null;
            isInitialized = false;
        }
        releaseAudioFocus();
    }

    private void releaseAudioFocus() {
        audioManager.abandonAudioFocus(null);
    }

    public interface InitCallback {
        void onSuccess();

        void onError(String error);
    }
}