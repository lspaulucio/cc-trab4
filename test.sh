#!/bin/bash

DATA=/home/leonardo/Documentos/Compiladores/Trabalhos/cc-trab2
TEST_FOLDER=/home/leonardo/Documentos/Compiladores/Trabalhos/Testes
IN=$TEST_FOLDER/in
OUT=$TEST_FOLDER/myout
CORRECT_OUT=$TEST_FOLDER/out2

EXE=./trab2
EXTENSION=.cm

for infile in `ls $IN/*$EXTENSION`;
do
    base=$(basename $infile)
    outfile=$OUT/${base/$EXTENSION/.out}
    $EXE < $infile > $outfile
done

for infile in `ls $IN/*$EXTENSION`
do
	base=$(basename $infile)
	myout=$OUT/${base/$EXTENSION/.out}
    outfile=$CORRECT_OUT/${base/$EXTENSION/.out}
	echo -e "\nTeste $base"
    if diff $myout $outfile
        then
            echo "OK"
        else
            echo "Diferente"
    fi
done
