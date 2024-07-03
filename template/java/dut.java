package com.ut;


import com.xspcomm.*;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.util.function.Consumer;


public class UT_{{__TOP_MODULE_NAME__}} {
    private static boolean LIB_LOADED = false;
    public static void loadLibraryFromJar(String path) throws IOException {
        InputStream inputStream = UT_{{__TOP_MODULE_NAME__}}.class.getResourceAsStream(path);
        if (inputStream == null) {
            throw new IOException("Could not find library: " + path);
        }
        File tempFile = File.createTempFile("lib", ".so");
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
    static {
        try {
            loadLibraryFromJar("/libUT{{__TOP_MODULE_NAME__}}.so");
            loadLibraryFromJar("/libUT_{{__TOP_MODULE_NAME__}}.so");
            xspcomm.init();
            LIB_LOADED = true;
        } catch (Exception e) {
            System.err.println("Error load so fail:");
            e.printStackTrace();
            LIB_LOADED = false;
        }
    }
    public static void main(String[] args) {
        System.out.printf("Dependency loaded: %b\n", LIB_LOADED);
    }
    public DutUnifiedBase dut;
    public XPort port;
    public XClock xclock;

    // all pins declare
{{__XDATA_DECL__}}
    private void initDut(){
        this.port = new XPort();
        this.xclock = new XClock(new CbXClockEval(dump -> this.dut.step(dump)));
	this.xclock.Add(this.port);
        // new pins
{{__XDATA_INIT__}}
        // bind dpi
{{__XDATA_BIND__}}
        // add to port
{{__XPORT_ADD__}}
    }
    public UT_{{__TOP_MODULE_NAME__}}(){
        this.dut = new DutUnifiedBase();
        this.initDut();
    }
    public UT_{{__TOP_MODULE_NAME__}}(String[] args){
        StringVector vec = new StringVector();
        for (int i = 0; i < args.length; i++) {
            vec.add(args[i]);
        }
        this.dut = new DutUnifiedBase(vec);
        this.initDut();
    }
    public void setWaveform(String wave_name){
        this.dut.set_waveform(wave_name);
    }
    public void setCoverage(String coverage_name){
        this.dut.set_coverage(coverage_name);
    }
    public void step(int i){
        this.xclock.Step(i);
    }
    public void step(){
        this.xclock.Step(1);
    }
    public void stepRis(Consumer<java.math.BigInteger> callback){
        this.xclock.StepRis(new CbXClockStep(callback));
    }
    public void stepFal(Consumer<java.math.BigInteger> callback){
        this.xclock.StepFal(new CbXClockStep(callback));
    }
    public void finished(){
        this.dut.finished();
    }
    public void initClock(String clock_name){
        this.xclock.Add(this.port.Get(clock_name));
    }
}

