
#include <mcv/mcv.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>
#include <cppast/visitor.hpp>

namespace mcv
{
    std::string get_template_args(std::string define){
        if(strconatins(define, "VerilatedTraceConfig")){
            return "";
        }
        if(!strconatins(define, "<")){
            return "";
        }
        return trim(strsplit(strsplit(define, "<").back(), ">").front());
    }

    class code_generator : public cppast::code_generator
    {
        std::string str_;          // the result
        bool was_newline_ = false; // whether or not the last token was a newline
        // needed for lazily printing them

    public:
        code_generator(const cppast::cpp_entity &e)
        {
            // kickoff code generation here
            cppast::generate_code(*this, e);
        }

        // return the result
        const std::string &str() const noexcept
        {
            return str_;
        }

    private:
        // called to retrieve the generation options of an entity
        generation_options do_get_options(const cppast::cpp_entity &,
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
    };

    void print_entity(std::ostream &out, const cppast::cpp_entity &e)
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
            auto &ns = static_cast<const cppast::cpp_namespace &>(e);
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
            code_generator generator(e);
            // print generated code
            out << ": `" << generator.str() << '`' << '\n';
        }
    }

    void print_cpp(std::unique_ptr<cppast::cpp_file> &cppfile)
    {
        std::string prefix;
        cppast::visit(*cppfile, [&](const cppast::cpp_entity &e, cppast::visitor_info info)
                      {
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
        return true; });
    }

    void print_cmembers(std::ostream &out, std::vector<CMember> &member)
    {
        for (auto &v : member)
        {
            out << v.pclass << "::[" << v.type << "]" << v.name << ": " << v.define << std::endl;
        }
    }

    std::vector<CMember> parse_cpp_public_items(std::unique_ptr<cppast::cpp_file> &cppfile, std::string &fname)
    {
        std::vector<CMember> ret;
        bool public_access = false;
        std::string current_class = "";
        cppast::visit(*cppfile, [&](const cppast::cpp_entity &e, cppast::visitor_info info)
                      {
        if (e.kind() == cppast::cpp_entity_kind::file_t || cppast::is_templated(e)
            || cppast::is_friended(e))
            return true;
        else if (info.event == cppast::visitor_info::container_entity_exit)
        {
        }
        else if (!e.name().empty())
        {
            code_generator g(e);
            mcv::CMember cmber{
                .pclass = "",
                .type = "",
                .name = e.name(),
                .define = streplace(g.str(), ";", ""),
            };
            // check verilator ex class
            for(auto &c: std::vector<std::string>{"VerilatedVcdC", "VerilatedTraceConfig",
                                                  "VlWide"}){
                if(strconatins(cmber.define, c)){
                    auto targs = get_template_args(cmber.define);
                    auto name = c;
                    if (!targs.empty()){
                        name = c + "<" + targs + ">";
                    }
                    mcv::CMember ex_class{
                        .pclass = c,
                        .type = "class",
                        .name = name,                  // class<T..>
                        .define = targs,     // T
                    };
                    ret.push_back(ex_class);
                }
            }
            if (e.kind() == cppast::cpp_entity_kind::include_directive_t){
                cmber.type = "inc";
                ret.push_back(cmber);
                return true;
            }
            if (e.kind() == cppast::cpp_entity_kind::class_t){
                if(cppast::is_definition(e)){
                    current_class = e.name();
                }else{
                    current_class = "";
                }
                public_access = false;
                return true;
            }
            if (e.kind() == cppast::cpp_entity_kind::access_specifier_t && !current_class.empty()){
                if (e.name() == "public"){
                    public_access = true;
                }else{
                    public_access = false;
                }
                return true;
            }
            // member var and functions
            if (public_access){
                if(e.kind() == cppast::cpp_entity_kind::member_function_t){
                    cmber.type = "fuc";
                    ret.push_back(cmber);
                    return true;
                }
                if (e.kind() == cppast::cpp_entity_kind::member_variable_t){
                    // Ignore some thing
                    if (strconatins(cmber.define, fname + "__")){
                        return true;
                    }
                    cmber.type = "var";
                    ret.push_back(cmber);
                    return true;
                }
            }
        }
        return true; });
        return ret;
    }
}
