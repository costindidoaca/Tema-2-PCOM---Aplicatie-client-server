Costin Didoaca
323CA

TEMA 2 Protocoale de Comunicatii

#Aplicatie client-server TCP si UDP pentru gestionarea mesajelor

    In aceasta tema, m-am folosit de cunostintele acumulate pe parcursul
laboratoarelor dar in special cel de Multiplexare I/O.

    Am creat in fisierul herlpers.h urmatoarele structuri: Packet, tcp_struct, 
udp_struct, topic si client. In structura Packet am vrut sa retin 
caracteristicile unui pachet de date transmis catre server/subscriber 
folositoare programului, in tcp_struct contine informatiile care se 
transmit unui anumit client cum ar fi: o variabila de tip bool care ne verifica 
daca un client este up sau nu util pentru continuarea conectarii la altul, un 
array de topicuri apartinand de fiecare client si un array de mesaje unent. In 
structura udp_struct retin informatiile incarcate de catre clientul udp cum ar 
fi continutul mesajului tipul si topicul. Ultima structura topic retine topicul extras si sfarsitul.

##Fisierele server.c si server_help_func.c

    Serverul gestioneaza un sistem in care clientii se pot abona sau dezabona de 
la anumite topicuri si pot primi actualizari despre topicurile respective 
prin intermediul retelei. Acesta proceseaza informatiile primite de la doua 
socket-uri TCP si UDP, proceseaza requesturile de abonare si dezabonare 
primitede la clienti si trimite mesaje corespunzatoare fiecarui client in 
functie de abonari. In main initializez socketurile, configurez descriptorii 
de fisiere si deschid un while care asteapta sa apara diverse actiuni cum ar 
fi primirea de mesaje, conectarea sau deconectarea clientilor verificarea 
daca serverul este on. 
    Initial serverul creeaza doi socketi, unul pentru conexiuni de tip UDP si 
celalalt pentru conexiuni TCP acestea fiind legate la aceeasi adresa si 
port. O functie se ocupa de mesajele primite prin UDP extragand toate 
informatiile relevante si trimitand mesajele catre clientii care au dat 
subscribe la subiectul respectiv. O alta functie se ocupa de conexiunile 
TCP, aceptand noi conexiuni de la clienti. Aceasta verifica daca cleintul 
exista deja in lista iar in caz contrar, proceseaza mesajele primite de la 
clientul verificat. Se gestioneaza si inputul de la tastatura si se inchide 
cand primeste comanda exit. Serverul se ocupa si de mesajele primite de la 
clientii TCP si actualizeaza lista de topicuri a clientului in functie de 
tipul mesajului primit (subscribe/unsubscribe).

#Fisierul subscribe.c

    Se initializeaza file descriptorii, se creeaza un socket TCP, se conecteaza la 
server si trimite modul de indentifdicare al clintului adica ID-ul acestuia.Se 
intra intr-un infinite loop care asteapta mesaje de la stdin si verifica daca 
s-a primit comanda care mai apoi, folosind informatiile atribuite comenzii, se 
introduce in pachetul trimis catre server. Comenzile pot fi pentru a inchide, 
a se abona sau dezabona de la un anumit topic. Se creeaza file descriptorul in 
care adaugam socketul si stdinul si se trimite ID-ul clientului catre server.

##Bibliografie

1. Laboratoare PCom [https://pcom.pages.upb.ro/labs/]
2. Videouri de Networking: [https://www.youtube.com/playlist?list=PLowKtXNTBypH19whXTVoG3oKSuOcw_XeW]
3. MUltiplexing I/O : [https://www.youtube.com/watch?v=dEHZb9JsmOU]

##Feedback

    O tema destul de provocatoare, gata sa-ti testeze abilitatile de programare cu noile 
cunostinte dobandite. Personal mi s-a parut interesanta, dezvoltandu-mi modul de a 
gandi modularizat, logic si algoritmic.
