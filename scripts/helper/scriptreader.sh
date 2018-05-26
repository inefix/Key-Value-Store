echo "creating reader"
cd ../..

./client << EOF
p
r 1
r 2
p
q
EOF
sleep 8
