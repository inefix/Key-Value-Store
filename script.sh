make clean
make 
echo "starting server:"
./server << EOF
a test1
a test2
a test3
p
EOF
make clean
