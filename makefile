SRC		=	main.c
OUT		=	BB_control
RM		=	rm -f
OBJ		=	$(SRC:.c=.o)

pi:
	gcc $(SRC) -o $(OUT) `chantilly --bp` -Wall -lm -fsigned-char

halt:
	gcc BBHALT_$(SRC) -o bbhalt `chantilly --bp` -Wall -lm -fsigned-char

dev:
	gcc x10DEV$(SRC) -o bbdev `chantilly --bp` -Wall -lm -fsigned-char
		
clean_main :
		$(RM) $(OBJ)
		$(RM) *~ \#*\#
		rm -f $(OUT)
clean_halt :
		$(RM) $(OBJ)
		$(RM) *~ \#*\#
		rm -f bbhalt
clean_dev :
		$(RM) $(OBJ)
		$(RM) *~ \#*\#
		rm -f bbdev