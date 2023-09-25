%{
#include "mvtype.h"
%}


%include "mvtype.h"

%typemap(gotype) (const char **) "[]string"
%typemap(imtype) (const char **) "uintptr"

%typemap(goin) (const char **) {
	if $input == nil || len($input) == 0 {
		$result = 0
	} else {
		$result = Alloc_ptr_array(uint(len($input) + 1))
		// defer func() {
		// 	Free_ptr_array($result)
		// }()
		var i uint
		for i = 0; i < uint(len($input)); i++ {
			Set_ptr_array($result, i, String_to_uintptr($input[i]))
		}
	}
}

%typemap(in) (const char **) {
    $1 = (char **) $input;
}