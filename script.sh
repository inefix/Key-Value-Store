make clean
make
echo "creating clients"

for VAR in {1..2}
do 
gnome-terminal -e ./client << EOF
a test1
a test2
a test3
p
q
EOF
	echo "starting client $VAR"
done

make clean
