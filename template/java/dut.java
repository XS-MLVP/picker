package com.ut.{{__TOP_MODULE_NAME__}};


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
        InputStream inputStream = UT_{{__TOP_MODULE_NAME__}}.class.getResourceAsStream(path);
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
            xspcomm.init();
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
    public DutUnifiedBase dut;
    public XPort xport;
    public XClock xclock;
    public XSignalCFG xcfg;
    private Map<String, XData> internalSignals = new java.util.HashMap<String, XData>();
    private Map<String, List<XData>> internalSignalsList = new java.util.HashMap<String, List<XData>>();

    // all pins declare
{{__XDATA_DECL__}}
    // Subports
{{ __XPORT_CASCADED_DEC__}}
    private void initDut(){
        this.xport = new XPort();
        this.xclock = new XClock(this.dut.getPxcStep(), this.dut.getPSelf());
        this.xclock.Add(this.xport);
        // init xcfg
        try {
            UT_{{__TOP_MODULE_NAME__}}.loadFileInJar("/{{__TOP_MODULE_NAME__}}_offset.yaml", (path -> {
                this.xcfg = new XSignalCFG(path, this.dut.GetXSignalCFGBasePtr());
            }));
        } catch (Exception e) {
            if(this.dut.GetXSignalCFGBasePtr().intValue() != 0) {
                System.err.println("Error load {{__TOP_MODULE_NAME__}}_offset.yaml fail:");
                e.printStackTrace();
            }else{
                this.xcfg = new XSignalCFG("/{{__TOP_MODULE_NAME__}}_offset.yaml", this.dut.GetXSignalCFGBasePtr());
            }
        }
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
        StringJavaVector vec = new StringJavaVector();
        for (int i = 0; i < args.length; i++) {
            vec.add(args[i]);
        }
        this.dut = new DutUnifiedBase(vec);
        this.initDut();
    }
    /*************************************************/
    /*                  User APIs                    */
    /*************************************************/
    public boolean ResumeWaveformDump(){
        return this.dut.ResumeWaveformDump();
    }
    public boolean PauseWaveformDump(){
        return this.dut.PauseWaveformDump();
    }
    public int WaveformPaused() {
        return this.dut.WaveformPaused();
    }
    public XClock GetXClock(){
        return this.xclock;
    }
    public XPort GetXPort(){
        return this.xport;
    }
    public void SetWaveform(String wave_name){
        this.dut.SetWaveform(wave_name);
    }
    public String GetWaveFormat() {
        return this.dut.GetWaveFormat();
    }
    public void FlushWaveform() {
        this.dut.FlushWaveform();
    }
    public void SetCoverage(String coverage_name){
        this.dut.SetCoverage(coverage_name);
    }
    public int GetCovMetrics() {
        return this.dut.GetCovMetrics();
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

    public List<String> VPIInternalSignalList() {
        return this.VPIInternalSignalList("");
    }
    public List<String> VPIInternalSignalList(String prefix) {
        return this.VPIInternalSignalList(prefix, 99);
    }
    public List<String> VPIInternalSignalList(String prefix, int deep) {
        List<String> vec = new ArrayList<>();
        this.dut.VPIInternalSignalList(prefix, deep).forEach((i) ->{
            vec.add((String)i);
        });
        return vec;
    }
    public XData GetInternalSignal(String name) {
        return this.GetInternalSignal(name, -1);
    }
    public XData GetInternalSignal(String name, int index) {
        return this.GetInternalSignal(name, index, false);
    }
    public XData GetInternalSignal(String name, int index, boolean use_vpi) {
        if (this.internalSignals.containsKey(name)) {
            return this.internalSignals.get(name);
        }
        XData signal = null;
        if(!use_vpi){
            String xname = "CFG:" + name;
            if (this.dut.GetXSignalCFGBasePtr().intValue() == 0) {
                return signal;
            }
            if (index >= 0) {
                signal = this.xcfg.NewXData(name, index, xname);
            }else{
                signal = this.xcfg.NewXData(name, xname);
            }
            if (signal != null) {
                this.internalSignals.put(name, signal);
            }
            return signal;
        }
        signal = XData.FromVPI(this.dut.GetVPIHandleObj(name),
                               this.dut.GetVPIFuncPtr("vpi_get"),
                               this.dut.GetVPIFuncPtr("vpi_get_value"),
                               this.dut.GetVPIFuncPtr("vpi_put_value"), "VPI:" + name);
        if (signal != null) {
            this.internalSignals.put(name, signal);
        }
        return signal;
    }
    public List<XData> GetInternalSignal(String name, Boolean is_array){
        if (this.internalSignalsList.containsKey(name)) {
            return this.internalSignalsList.get(name);
        }
        List<XData> result = new ArrayList<>();
        if (this.dut.GetXSignalCFGBasePtr().intValue() == 0) {
            return result;
        }
        if(!is_array){
            return result;
        }
        String xname = "CFG:" + name;
        XDataVector signalList = this.xcfg.NewXDataArray(name, xname);
        if (signalList != null) {
            signalList.forEach((i) -> {
                if (i != null) {
                    result.add((XData)i);
                }
            });
        }
        return result;
    }
    public List<String> GetInternalSignalList(){
        return this.GetInternalSignalList("");
    }
    public List<String> GetInternalSignalList(String prefix){
        return this.GetInternalSignalList(prefix, 99);
    }
    public List<String> GetInternalSignalList(String prefix, Integer deep){
        return this.GetInternalSignalList(prefix, deep, false);
    }
    public List<String> GetInternalSignalList(String prefix, Integer deep, Boolean use_vpi){
        List<String> ret = new ArrayList<>();
        if (this.dut.GetXSignalCFGBasePtr().intValue() != 0 && !use_vpi) {
            this.xcfg.GetSignalNames(prefix).forEach ((i) -> {
                if (i != null) {
                    if (prefix == "" || i.startsWith(prefix)) {
                        ret.add((String)i);
                    }
                }
            });
            return ret;
        }
        return this.VPIInternalSignalList(prefix, deep);
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
    public void AtClone() {
        this.dut.atClone();
    }
    /*************************************************/
    /*               End of User APIs                */
    /*************************************************/
}
