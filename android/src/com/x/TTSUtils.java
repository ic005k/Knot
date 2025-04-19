package com.x;

import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import android.widget.Toast;
import java.util.Locale;

public class TTSUtils extends UtteranceProgressListener {
  private Context mContext;
  private static TTSUtils singleton;
  private TextToSpeech textToSpeech; // 系统语音播报类
  private boolean isSuccess = true;

  public static TTSUtils getInstance(Context context) {
    if (singleton == null) {
      synchronized (TTSUtils.class) {
        if (singleton == null) {
          singleton = new TTSUtils(context);
        }
      }
    }
    return singleton;
  }

  private TTSUtils(Context context) {
    this.mContext = context.getApplicationContext();
    textToSpeech =
      new TextToSpeech(
        mContext,
        i -> {
          //系统语音初始化成功
          if (i == TextToSpeech.SUCCESS) {
            int result = textToSpeech.setLanguage(Locale.CHINA);
            textToSpeech.setPitch(2.0f); // 设置音调，值越大声音越尖（女生），值越小则变成男声,1.0是常规
            textToSpeech.setSpeechRate(1.0f);
            textToSpeech.setOnUtteranceProgressListener(TTSUtils.this);
            if (
              result == TextToSpeech.LANG_MISSING_DATA ||
              result == TextToSpeech.LANG_NOT_SUPPORTED
            ) {
              //系统不支持中文播报
              isSuccess = false;
            }
          }
        }
      ); //百度的播放引擎 "com.baidu.duersdk.opensdk"
  }

  public void playText(String playText) {
    if (!isSuccess) {
      Toast.makeText(mContext, "系统不支持中文播报", Toast.LENGTH_SHORT).show();
      return;
    }

    if (textToSpeech != null) {
      textToSpeech.speak(playText, TextToSpeech.QUEUE_ADD, null, null);
    }
  }

  public void stopSpeak() {
    if (textToSpeech != null) {
      textToSpeech.stop();
    }
  }

  public void shutdownSpeak() {
    textToSpeech.shutdown();
  }

  @Override
  public void onStart(String utteranceId) {}

  @Override
  public void onDone(String utteranceId) {}

  @Override
  public void onError(String utteranceId) {}
}
