cd ..
echo "cleaning"
make clean
echo "compiling"
make
cd scripts
echo "launching the scripts"

gnome-terminal -x sh -c "./scriptserver.sh"
sleep 1 # let the server start
# gnome-terminal -x sh -c "./scriptwriter.sh"
sleep 5
gnome-terminal -x sh -c "./scriptreader.sh"
sleep 10
cd ..
echo "cleaning"
make clean
echo "script terminated correctly"
