#!/bin/bash

# 检查是否提供了宏名称
if [ $# -ne 2 ]; then
    echo "Usage: $0 <file_path> <macro_name>"
    exit 1
fi

file_path=$1
macro_name=$2

# 使用 grep 和 sed 查找宏的值
value=$(grep -E "\`define\s+$macro_name\s+\S+" $file_path | sed -E "s/\`define\s+$macro_name\s+(\S+)/\1/")

# 检查是否找到宏定义并输出结果
if [ -n "$value" ]; then
    echo $value
else
    echo "Macro \`$macro_name not found"
fi
