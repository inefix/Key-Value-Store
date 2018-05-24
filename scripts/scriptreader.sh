echo "creating reader"
cd ..

./client << EOF
p
r 1 
r 3
p
q
EOF
sleep 8
