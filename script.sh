make clean
make
echo "starting server:"
./server << EOD
a test1
a test2
a test3
p
q
EOD
make clean
