# 2 READERS at the same time
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

for I in {1..50}
do
	gnome-terminal -x sh -c "./scriptreader.sh"
	sleep 0.1
done
sleep 10
cd ..
echo "cleaning"
make clean
echo "script terminated correctly"
