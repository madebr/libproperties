#!/usr/bin/bash -x


SELFDIR=$(cd $(dirname $0);pwd)



function    compile_java_tools()
{
    rm -rf  ${SELFDIR}/*.class
    cd      "${SELFDIR}" && javac "${SELFDIR}/Main.java"
    RESULT=$?
    if [ ${RESULT} -ne 0 ] || [ ! -f ${SELFDIR}/Main.class ]; then
        echo    "Compile the java test tool failed"
        return  1
    fi 

    return  0
}



function    compile_c_tools()
{
    #rm -rf  ${SELFDIR}/*.class
    #cd      "${SELFDIR}" && javac "${SELFDIR}"
    #RESULT=$?
    #if [ ${RESULT} -ne 0 ] || [ ! -f ${SELFDIR}/*.class ]; then
    #    echo    "Compile the java test tool failed"
    #    return  1
    #fi 

    return  0
}




function    generate_expect_files()
{
    local files=$(ls | grep -E '[0-9]+\.properties$')
    for file in ${files}; do
        rm -rf  "${file}.java.expect"
        cd      "${SELFDIR}" && java -cp "./" Main "${file}" > "${file}.java.expect"
        RESULT=$?
        if [ ${RESULT} -ne 0 ] || [ ! -f "${file}.java.expect" ]; then
            echo    "Create expect file failed: ${file}"
            return  1
        fi 
    done

    return  0
}




function    run_teatcases()
{
    local exe_tool=${SELFDIR}/$(cd ${SELFDIR};ls | grep liproperties_test)
    if [ ! -f "${exe_tool}" ]; then
        echo    "The C test tool is not exist"
        return  3
    fi

    files=$(ls -al | grep -E '[0-9]+\.properties$')
    for file in $(files); do
        rm -rf  "${file}.c.expect"
        ${exe_tool} "${file}"
        RESULT=$?
        if [ ${RESULT} -ne 0 ] || [ ! -f "${file}.c.expect" ]; then
            echo    "Parse file failed: ${file}"
            return  4
        fi
    done

    return  0
}




function main()
{
    echo    "Compile java test tool ..."
    compile_java_tools
    RESULT=$?
    if [ $RESULT -ne 0 ]; then
        echo    "Compile java test tool failed"
        return  1
    fi
    echo    "Compile java test tool success"


    echo    "Compile c test tool ..."
    compile_c_tools
    RESULT=$?
    if [ $RESULT -ne 0 ]; then
        echo    "Compile c test tool failed"
        return  2
    fi
    echo    "Compile c test tool success"


    echo    "Generate expect files ..."
    generate_expect_files
    RESULT=$?
    if [ $RESULT -ne 0 ]; then
        echo    "Generate expect files failed"
        return  2
    fi
    echo    "Generate expect files success"


    echo    "Run testcases ..."
    run_teatcases
    RESULT=$?
    if [ $RESULT -ne 0 ]; then
        echo    "Generate expect files failed"
        return  2
    fi
    echo    "Run testcases success"


    return  0
}



main    "$@"
exit    "$?"