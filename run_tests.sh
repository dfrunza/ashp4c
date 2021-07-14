reset;
for f in `find testdata -maxdepth 1 -type f`; do \
    echo;
    ./build/ashp4c $f;
    if [ $? -eq 0 ]; then
        echo "--------";
        echo "$f : PASSED"
    else
        echo "$f : FAILED"
    fi
done
