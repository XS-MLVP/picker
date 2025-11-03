package com.ut.{{__TOP_MODULE_NAME__}};

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.util.function.Consumer;

import com.xspcomm.*;

public class JavaUT_{{__TOP_MODULE_NAME__}} { //this class is used for scala extendtion
    private static boolean LIB_LOADED = false;
    public static void loadLibraryFromJar(String path) throws IOException {
        InputStream inputStream = JavaUT_{{__TOP_MODULE_NAME__}}.class.getResourceAsStream(path);
        if (inputStream == null) {
            throw new IOException("Could not find library: " + path);
        }
        File tempFile = File.createTempFile("lib", ".{{__SHARED_LIB_SUFFIX__}}");
        tempFile.deleteOnExit();
        try (FileOutputStream outputStream = new FileOutputStream(tempFile)) {
            byte[] buffer = new byte[1024];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }
        }
        System.load(tempFile.getAbsolutePath());
    }
    public static void loadFileInJar(String path, Consumer<String> cb) throws IOException {
        InputStream inputStream = JavaUT_{{__TOP_MODULE_NAME__}}.class.getResourceAsStream(path);
        if (inputStream == null) {
            throw new IOException("Could not find file: " + path);
        }
        File tempFile = File.createTempFile("tmp", "tmp");
        tempFile.deleteOnExit();
        try (FileOutputStream outputStream = new FileOutputStream(tempFile)) {
            byte[] buffer = new byte[1024];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }
        }
        cb.accept(tempFile.getAbsolutePath());
    }
    static {
        try {
            loadLibraryFromJar("/libUT{{__TOP_MODULE_NAME__}}.{{__SHARED_LIB_SUFFIX__}}");
            loadLibraryFromJar("/libUT_{{__TOP_MODULE_NAME__}}.{{__JNI_LIB_SUFFIX__}}");
            xspcommLoader.init();
            LIB_LOADED = true;
        } catch (Exception e) {
            System.err.println("Error load native/shared library fail:");
            e.printStackTrace();
            LIB_LOADED = false;
        }
    }
    public static void main(String[] args) {
        System.out.printf("Dependency loaded: %b\n", LIB_LOADED);
    }
}

