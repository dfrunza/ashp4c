reset;
for f in `ls testdata`; do \
    echo;
    ./build/dp4c testdata/$f;
    if [ $? -eq 0 ]; then
        echo "$f : PASSED"
    else
        echo "$f : FAILED"
    fi
done
