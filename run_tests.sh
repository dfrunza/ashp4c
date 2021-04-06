reset;
for f in `ls testdata`; do \
      echo;
      echo " -- $f --";
      ./build/dp4c testdata/$f;
done
