# a reader and a writer comes in when reader is reading
cd ..
echo "cleaning"
make clean
echo "compiling"
make
cd scripts/helper
echo "launching the scripts"

gnome-terminal -x sh -c "./scriptserver.sh"
gnome-terminal -x sh -c "./fillkv.sh"
sleep 5 # let the server start
gnome-terminal -x sh -c "./scriptreader.sh"
sleep 1
gnome-terminal -x sh -c "./scriptwriter.sh"
sleep 10
cd ../..
echo "cleaning"
make clean
echo "script terminated correctly"
