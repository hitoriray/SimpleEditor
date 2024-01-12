import os
import lorem
import subprocess


def generate_input_file(input_file, num_lines, max_line_length=320):
    with open(input_file, 'w') as file:
        lines = []
        for _ in range(num_lines):
            line = lorem.paragraph()
            if len(line) > max_line_length:
                line = line[:max_line_length]  # 截断超过最大长度的部分
            lines.append(line + "\n")

        file.writelines(lines)


def compile_simple_editor():
    compile_command = "g++ simple_editor.cpp -o ../bin/simple_editor"
    subprocess.run(compile_command, shell=True)


def run_simple_editor(input_file, output_file):
    if not os.path.exists(output_file):
        create_output_file(output_file)
    subprocess.run(["../bin/simple_editor", input_file, output_file])


def create_output_file(output_file):
    with open(output_file, 'w'):
        pass  # 创建一个空文件


if __name__ == "__main__":
    # 在脚本开始处进行一次编译即可
    compile_simple_editor()
    # 测试数据
    for i, num_lines in enumerate([50, 100, 150, 200, 250], start=1):
        for j, line_length in enumerate([50, 80, 160, 240, 320], start=1):
            input_file = f"../data/input/input_{i}.txt"
            output_file = f"../data/output/output_{i}.txt"
            if i == j:
                generate_input_file(input_file, num_lines, line_length)
                print(f"Generated {input_file} with {num_lines} lines({line_length} per line).")
                # 调用C++程序进行编辑
                run_simple_editor(input_file, output_file)
                print(f"C++程序执行完成。输出文件：{output_file}")
