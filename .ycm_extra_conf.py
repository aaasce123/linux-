#!/usr/bin/env python
# coding=utf-8
import os
import sys

def DirectoryOfThisScript():
    return os.path.dirname(os.path.abspath(__file__))

def Settings(**kwargs):
    return {
        'flags': [
            '-x', 'c',                  # 指定为 C 文件（不是 C++）
            '-std=c11',                # 使用 C11 标准
            '-Wall',                   # 打开所有警告
            '-I', 'include',           # 添加头文件路径（相对路径）
            '-I', 'src',               # 如需要，也可添加源文件路径
        ],
        'include_paths_relative_to_dir': DirectoryOfThisScript(),
        'override_filename': kwargs.get('filename', 'main.c'),  # 防止空文件触发错误
    }

