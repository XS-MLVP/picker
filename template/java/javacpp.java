import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;

@Platform(include="{{dut_name}}.h", 
          link="{{dut_name}}",
          includepath={"{{verilator_include}}","{{verilator_include}}/vltstd"},
          linkpath={"{{work_java_dir}}"})
public class {{dut_name}} {
    static { Loader.load(); }
    
    public static void main(String[] args) {
        System.out.println("Start test");
    }
}
