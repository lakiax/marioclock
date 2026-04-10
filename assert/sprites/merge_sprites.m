% merge_sprites.m - 万能版 LVGL C 数组合并脚本
clear; clc;

% 1. 读取 maping.txt 动作描述 (如果存在)
map_dict = containers.Map('KeyType', 'char', 'ValueType', 'char');
if isfile('maping.txt')
    fid = fopen('maping.txt', 'r');
    while ~feof(fid)
        line = fgetl(fid);
        if ischar(line)
            tokens = regexp(line, '([a-zA-Z0-9-]+),(.+)', 'tokens');
            if ~isempty(tokens)
                map_dict(tokens{1}{1}) = tokens{1}{2};
            end
        end
    end
    fclose(fid);
end

h_file = fopen('mario_sprites.h', 'w', 'n', 'UTF-8');
c_file = fopen('mario_sprites.c', 'w', 'n', 'UTF-8');

fprintf(h_file, '#pragma once\n#include <stdint.h>\n\n');
fprintf(c_file, '#include "mario_sprites.h"\n\n');

% 2. 遍历所有导出的 .c 资源文件
files = dir('sprite-*.c');
for i = 1:length(files)
    filename = files(i).name;
    content = fileread(filename);
    
    % 使用强大的正则提取：匹配 uint8_t 变量名[] = { 数据 };
    tokens = regexp(content, 'uint8_t\s+([a-zA-Z0-9_-]+)\[\]\s*=\s*\{([^}]+)\}', 'tokens');
    
    if ~isempty(tokens)
        raw_name = tokens{1}{1}; % 例如 sprite-7-6_map
        data_str = tokens{1}{2}; % 数组内部的全部 hex 数据
        
        % 替换 C 语言非法的减号
        safe_name = strrep(raw_name, '-', '_'); 
        
        % 提取纯 ID 用于匹配注释
        id_tokens = regexp(raw_name, 'sprite-([a-zA-Z0-9-]+)_map', 'tokens');
        comment = '图集或序列帧';
        if ~isempty(id_tokens) && isKey(map_dict, id_tokens{1}{1})
            comment = map_dict(id_tokens{1}{1});
        end
        
        % 写入 .h 文件（隐式大小声明，完美兼容任何像素的图）
        fprintf(h_file, '// %s\n', comment);
        fprintf(h_file, 'extern const uint8_t %s[];\n\n', safe_name);
        
        % 写入 .c 文件
        fprintf(c_file, '// %s\n', comment);
        fprintf(c_file, 'const uint8_t %s[] = {\n%s\n};\n\n', safe_name, strtrim(data_str));
    end
end

fclose(h_file);
fclose(c_file);
disp('🎉 解析合并完成！已生成支持动态大小的 mario_sprites.h 和 .c 文件。');