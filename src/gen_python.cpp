#include <mcv/mcv.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>
#include <cppast/visitor.hpp>


void print_entity(std::ostream& out, const cppast::cpp_entity& e)
{
    // print name and the kind of the entity
    if (!e.name().empty())
        out << e.name();
    else
        out << "<anonymous>";
    out << " (" << cppast::to_string(e.kind()) << ")";

    // print whether or not it is a definition
    if (cppast::is_definition(e))
        out << " [definition]";

    // print number of attributes
    if (!e.attributes().empty())
        out << " [" << e.attributes().size() << " attribute(s)]";

    if (e.kind() == cppast::cpp_entity_kind::language_linkage_t)
        // no need to print additional information for language linkages
        out << '\n';
    else if (e.kind() == cppast::cpp_entity_kind::namespace_t)
    {
        // cast to cpp_namespace
        auto& ns = static_cast<const cppast::cpp_namespace&>(e);
        // print whether or not it is inline
        if (ns.is_inline())
            out << " [inline]";
        out << '\n';
    }
    else
    {
        // print the declaration of the entity
        // it will only use a single line
        // derive from code_generator and implement various callbacks for printing
        // it will print into a std::string
        class code_generator : public cppast::code_generator
        {
            std::string str_;                 // the result
            bool        was_newline_ = false; // whether or not the last token was a newline
            // needed for lazily printing them

        public:
            code_generator(const cppast::cpp_entity& e)
            {
                // kickoff code generation here
                cppast::generate_code(*this, e);
            }

            // return the result
            const std::string& str() const noexcept
            {
                return str_;
            }

        private:
            // called to retrieve the generation options of an entity
            generation_options do_get_options(const cppast::cpp_entity&,
                                              cppast::cpp_access_specifier_kind) override
            {
                // generate declaration only
                return code_generator::declaration;
            }

            // no need to handle indentation, as only a single line is used
            void do_indent() override {}
            void do_unindent() override {}

            // called when a generic token sequence should be generated
            // there are specialized callbacks for various token kinds,
            // to e.g. implement syntax highlighting
            void do_write_token_seq(cppast::string_view tokens) override
            {
                if (was_newline_)
                {
                    // lazily append newline as space
                    str_ += ' ';
                    was_newline_ = false;
                }
                // append tokens
                str_ += tokens.c_str();
            }

            // called when a newline should be generated
            // we're lazy as it will always generate a trailing newline,
            // we don't want
            void do_write_newline() override
            {
                was_newline_ = true;
            }

        } generator(e);
        // print generated code
        out << ": `" << generator.str() << '`' << '\n';
    }
}


void gen_python(inja::json &cfg, std::unique_ptr<cppast::cpp_file> &cppfile, std::string target_dir){
    std::string prefix;
    cppast::visit(*cppfile, [&](const cppast::cpp_entity& e, cppast::visitor_info info) {
        if (e.kind() == cppast::cpp_entity_kind::file_t || cppast::is_templated(e)
            || cppast::is_friended(e))
            // no need to do anything for a file,
            // templated and friended entities are just proxies, so skip those as well
            // return true to continue visit for children
            return true;
        else if (info.event == cppast::visitor_info::container_entity_exit)
        {
            // we have visited all children of a container,
            // remove prefix
            prefix.pop_back();
            prefix.pop_back();
        }
        else
        {
            std::cout << prefix; // print prefix for previous entities
            // calculate next prefix
            if (info.last_child)
            {
                if (info.event == cppast::visitor_info::container_entity_enter)
                    prefix += "  ";
                std::cout << "+-";
            }
            else
            {
                if (info.event == cppast::visitor_info::container_entity_enter)
                    prefix += "| ";
                std::cout << "|-";
            }
            print_entity(std::cout, e);
        }
        return true;
    });

    // Gen pybind11
    auto fname = cfg[CFG_MODEL_NAME].template get<std::string>();
    auto vrinc = cfg[CFG_VERILATOR_INCLUDE].template get<std::string>();
    auto pyccp = path_join({target_dir, fname + "_python.cpp"});
    auto pybsh = path_join({target_dir, fname + "_python.build"});

    auto data = std::map<std::string,std::string>{
        {"dut_name", fname},
        {"mode_name","mcv"},
        {"body",""},
    };
    auto pytmp = find_file(std::vector<std::string>{"template/python/pybind11.cpp", 
                                                    "/etc/mcv/template/python/pybind11.cpp"});
    write_file(pyccp, template_rander(pytmp, data));

    auto build_file = fname + "__ALL.cpp " + fname + "_python.cpp";
    auto build_opts = std::string("-faligned-new -faligned-new -O3 -Wall -shared -std=c++17 -fPIC");
    auto build_python = "cd `dirname $0`\nc++ "+build_opts+" $(python3 -m pybind11 --includes) -I " + vrinc +
    " -o mcv$(python3-config --extension-suffix) " + vrinc + "/verilated.cpp "+ build_file +"\n";
    
    write_file(pybsh, build_python);
    MESSAGE("build python ...");
    exec_result("bash " + pybsh, 1);
}
