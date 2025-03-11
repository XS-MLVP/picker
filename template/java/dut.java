package com.ut;


import com.xspcomm.*;
import java.io.File;
import java.util.*;
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
    public XPort xport;
    public XClock xclock;
    private Map<String, XData> internalSignals = new java.util.HashMap<String, XData>();

    // all pins declare
{{__XDATA_DECL__}}
    // Subports
{{ __XPORT_CASCADED_DEC__}}
    private void initDut(){
        this.xport = new XPort();
        this.xclock = new XClock(this.dut.getPxcStep(), this.dut.getPSelf());
        this.xclock.Add(this.xport);
        // new pins
{{__XDATA_INIT__}}
        // bind dpi
{{__XDATA_BIND__}}
        // add to port
{{__XPORT_ADD__}}
        // new subports
{{__XPORT_CASCADED_SGN__}}
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
    /*************************************************/
    /*                  User APIs                    */
    /*************************************************/
    public void SetWaveform(String wave_name){
        this.dut.SetWaveform(wave_name);
    }
    public void FlushWaveform() {
        this.dut.FlushWaveform();
    }
    public void SetCoverage(String coverage_name){
        this.dut.SetCoverage(coverage_name);
    }
    public void Step(int i){
        this.xclock.Step(i);
    }
    public void Step(){
        this.xclock.Step(1);
    }
    public void StepRis(Consumer<Long> callback){
        this.xclock.StepRis(callback);
    }
    public void StepFal(Consumer<Long> callback){
        this.xclock.StepFal(callback);
    }

    public Integer CheckPoint(String check_point) {
        return this.dut.CheckPoint(check_point);
    }

    public Integer Restore(String check_point) {
        return this.dut.Restore(check_point);
    }

    public StringVector VPIInternalSignalList(String prefix, int deep) {
        return this.dut.VPIInternalSignalList(prefix, deep);
    }

    public XData GetInternalSignal(String name) {
        if (this.internalSignals.containsKey(name)) {
            return this.internalSignals.get(name);
        }
        XData data = XData.FromVPI(dut.GetVPIHandleObj(name),
                dut.GetVPIFuncPtr("vpi_get"),
                dut.GetVPIFuncPtr("vpi_get_value"),
                dut.GetVPIFuncPtr("vpi_put_value"), name);
        this.internalSignals.put(name, data);
        return data;
    }

    public void Finish(){
        this.dut.Finish();
    }
    public void InitClock(String clock_name){
        this.xclock.Add(this.xport.Get(clock_name));
    }
    public void RefreshComb(){
        this.dut.RefreshComb();
    }
    /*************************************************/
    /*               End of User APIs                */
    /*************************************************/
}

