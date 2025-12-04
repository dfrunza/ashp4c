for f in `find testdata -maxdepth 1 -type f -name "*.p4"`; do \
    echo;
    ./cmake-build-debug/ashp4c $f;
    if [ $? -eq 0 ]; then
        echo "$f ... [PASS]"
        echo "--------";
    else
        echo "$f ... [FAIL]"
        echo "--------";
    fi
done
