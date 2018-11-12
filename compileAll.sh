echo Compiling all
cd CircuitRouter-ParSolver
make clean
make
cd ../CircuitRouter-SeqSolver
make
cd ..
echo Done!
