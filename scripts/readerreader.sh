# 2 READERS at the same time
cd ..
echo "cleaning"
make clean
echo "compiling"
make
cd scripts
echo "launching the scripts"
gnome-terminal -x sh -c "./scriptserver.sh"
gnome-terminal -x sh -c "./fillkv.sh"
sleep 5 # let the server start
gnome-terminal -x sh -c "./scriptreader.sh"
gnome-terminal -x sh -c "./scriptreader.sh"
sleep 10
cd ..
echo "cleaning"
make clean
echo "script terminated correctly"
