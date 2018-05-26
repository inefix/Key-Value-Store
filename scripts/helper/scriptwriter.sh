echo "creating writer"
cd ../..
./client << EOF
p
a test6
m 1 test7
d 2
p
q
EOF
sleep 8
