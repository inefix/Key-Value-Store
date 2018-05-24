make clean
make
echo "launching the scripts"

gnome-terminal -e ./scriptserver.sh
gnome-terminal -e ./scriptreader.sh
gnome-terminal -e ./clientwriter.sh


make clean
