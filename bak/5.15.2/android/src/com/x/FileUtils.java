package com.x;

import java.nio.file.Files;
import android.annotation.SuppressLint;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.os.Build.VERSION;
import android.provider.DocumentsContract;
import android.provider.MediaStore.Images.Media;
import android.text.TextUtils;
import android.util.Log;
import androidx.annotation.NonNull;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.Locale;

public class FileUtils {
    private static final String TAG = "FileUtils";
    private static SimpleDateFormat sf = new SimpleDateFormat("yyyyMMdd_HHmmssSS");

    private FileUtils() {
    }

    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    public static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }

    public static String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
        Cursor cursor = null;
        String column = "_data";
        String[] projection = new String[] { "_data" };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs, (String) null);
            if (cursor != null && cursor.moveToFirst()) {
                int column_index = cursor.getColumnIndexOrThrow("_data");
                String var8 = cursor.getString(column_index);
                return var8;
            }
        } catch (IllegalArgumentException var12) {
            Log.i("FileUtils", String.format(Locale.getDefault(), "getDataColumn: _data - [%s]", var12.getMessage()));
        } finally {
            if (cursor != null) {
                cursor.close();
            }

        }

        return null;
    }

    @SuppressLint({ "NewApi" })
    public static String getPath(Context context, Uri uri) {
        boolean isKitKat = VERSION.SDK_INT >= 19;
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            String id;
            String[] split;
            String type;
            if (isExternalStorageDocument(uri)) {
                id = DocumentsContract.getDocumentId(uri);
                split = id.split(":");
                type = split[0];
                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }
            } else if (isDownloadsDocument(uri)) {
                id = DocumentsContract.getDocumentId(uri);
                if (!TextUtils.isEmpty(id)) {
                    try {
                        Uri contentUri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"),
                                Long.valueOf(id));
                        return getDataColumn(context, contentUri, (String) null, (String[]) null);
                    } catch (NumberFormatException var9) {
                        Log.i("FileUtils", var9.getMessage());
                        return null;
                    }
                }
            } else if (isMediaDocument(uri)) {
                id = DocumentsContract.getDocumentId(uri);
                split = id.split(":");
                type = split[0];
                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = android.provider.MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = android.provider.MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                String selection = "_id=?";
                String[] selectionArgs = new String[] { split[1] };
                return getDataColumn(context, contentUri, "_id=?", selectionArgs);
            }
        } else {
            if ("content".equalsIgnoreCase(uri.getScheme())) {
                if (isGooglePhotosUri(uri)) {
                    return uri.getLastPathSegment();
                }

                return getDataColumn(context, uri, (String) null, (String[]) null);
            }

            if ("file".equalsIgnoreCase(uri.getScheme())) {
                return uri.getPath();
            }
        }

        return null;
    }

    public static boolean copyFile(FileInputStream fileInputStream, String outFilePath) throws IOException {
        if (fileInputStream == null) {
            return false;
        } else {
            FileChannel inputChannel = null;
            FileChannel outputChannel = null;

            boolean var5;
            try {
                inputChannel = fileInputStream.getChannel();
                outputChannel = (new FileOutputStream(new File(outFilePath))).getChannel();
                inputChannel.transferTo(0L, inputChannel.size(), outputChannel);
                inputChannel.close();
                boolean var4 = true;
                return var4;
            } catch (Exception var9) {
                var5 = false;
            } finally {
                if (fileInputStream != null) {
                    fileInputStream.close();
                }

                if (inputChannel != null) {
                    inputChannel.close();
                }

                if (outputChannel != null) {
                    outputChannel.close();
                }

            }

            return var5;
        }
    }

    public static void copyFile(@NonNull String pathFrom, @NonNull String pathTo) throws IOException {
        if (!pathFrom.equalsIgnoreCase(pathTo)) {
            FileChannel outputChannel = null;
            FileChannel inputChannel = null;

            try {
                inputChannel = (new FileInputStream(new File(pathFrom))).getChannel();
                outputChannel = (new FileOutputStream(new File(pathTo))).getChannel();
                inputChannel.transferTo(0L, inputChannel.size(), outputChannel);
                inputChannel.close();
            } finally {
                if (inputChannel != null) {
                    inputChannel.close();
                }

                if (outputChannel != null) {
                    outputChannel.close();
                }

            }

        }
    }

    public static String getCreateFileName(String prefix) {
        long millis = System.currentTimeMillis();
        return prefix + sf.format(millis);
    }

    public static String getCreateFileName() {
        long millis = System.currentTimeMillis();
        return sf.format(millis);
    }

    public static String rename(String fileName) {
        String temp = fileName.substring(0, fileName.lastIndexOf("."));
        String suffix = fileName.substring(fileName.lastIndexOf("."));
        return temp + "_" + getCreateFileName() + suffix;
    }
}
