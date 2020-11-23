exec: 
		#construindo o projeto
		gcc conexao.c -c -g -pthread
		gcc manipulaSocketImg.c -c -g -pthread
		gcc manipulaSocketTxt.c -c -g -pthread
		gcc fila.c -c -g -pthread
		gcc servidor2.c conexao.o manipulaSocketImg.o manipulaSocketTxt.o fila.o -o servidor -g -pthread
clear:
		#limpando
		rm conexao.o
		rm manipulaSocketImg.o
		rm manipulaSocketTxt.o
		rm fila.o
		rm servidor
