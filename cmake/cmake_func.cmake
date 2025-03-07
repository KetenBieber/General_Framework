function(check_and_add_flags flags variable)
    foreach(flag IN LISTS flags)
        string(FIND "${${variable}}" "${flag}" flag_found)
        if(flag_found EQUAL -1)
            set(flag_present FALSE PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(flag_present TRUE PARENT_SCOPE)
endfunction()

# 递归包含头文件的函数
function(include_sub_directories_recursively root_dir)
    if (IS_DIRECTORY ${root_dir})               # 当前路径是一个目录吗，是的话就加入到包含目录
        message("include dir: " ${root_dir})
        include_directories(${root_dir})
    endif()

    file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*) # 获得当前目录下的所有文件，让如ALL_SUB列表中
    foreach(sub ${ALL_SUB})
        if (IS_DIRECTORY ${root_dir}/${sub})
            include_sub_directories_recursively(${root_dir}/${sub}) # 对子目录递归调用，包含
        endif()
    endforeach()
endfunction()