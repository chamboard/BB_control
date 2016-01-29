SRC		=	main.c
OUT		=	BB_control
RM		=	rm -f
OBJ		=	$(SRC:.c=.o)
#
# crude mkf
#
# control = setup 0xBB battery refill / stop refill levels ex: BB_control 7.8 8.2
control:
	gcc $(SRC) -o $(OUT) `chantilly --bp` -Wall -lm -fsigned-char
#
# not cooked again
# ex : bbhalt 1 10   
# means halt 0xBB+host (raspberry ..) , allow 10 second before executing that.
halt:
	gcc BBHALT_$(SRC) -o bbhalt `chantilly --bp` -Wall -lm -fsigned-char
#
# beta dev new functions on io24 0x10 board. firmware rev6 , added funcs 0x0a,0x0b,0x0c,0x0d to the firmware
#
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