Γενικά:
    - Για το compilation χρειάζονται οι εντολές:
        1. make clean
        2. make
    - Στα test όπου είχε «issueJob progDelay», το έκανα «issueJob ./progDelay»,
    παρατήρησα ότι ενώ και τα δύο τρέχουν το 2ο είναι πιο stable.
    - Στο «test_sh_scripts_2.sh» άλλαξα την γραμμή 2 από 
        «./jobCommander issueJob setConcurrency 4» σε
        «./jobCommander setConcurrency 4», καθώς το πρώτο είναι λάθος.
    - Λόγω μερικής λειτουργίας του «test_sh_scripts_1.sh» το ίδιο τρέχει 
    μέχρι ένα σημείο αλλά μετά κρασάρει.
    - Για το τελευταίο λόγο, μάλλον είναι καλύτερο να τρέξει πρώτα το 
    «test_sh_scripts_2.sh».

1. Φτιάχτηκε Queue Structure πλήρως από μένα και αποτελείται από δύο αρχεία:
    - queue.c: Ο κώδικας των συναρτήσεων, του Queue και του Triplet (struct 
    για τις τριπλέτες των job), τα οποία περνάνε ως Value στα Node του Queue.
    - queue.h: Ο ορισμός των συναρτήσεων και των struct, του Queue και του Triplet.

2. Το αρχείο jobCommander.c κάνει όλες τις λειτουργίες του jobCommander και αυτές 
μέσω της συνάρτησης «int jobCommander(int argc, char *argv[])».
    - Ελέγχει αν ο Server είναι ενεργός και αν όχι εκτελεί το «./jobExecutorServer»
    για να τον ανοίξει.
    - Ανοίγει το fifo pipe για γράψιμο του Commander και διάβασμα του Server.
    - Γράφει στο pipe χρήσιμες πληροφορίες για τη μεταφορά μηνυμάτων από και προς
    τον Server (πχ: Commander pid, πλήθος arguments).
    - Ελέγχει αν υπάρχουν πολλά arguments και τότε τα στέλνει σε πακέτα. Σε αυτή
    τη περίπτωση γίνεται και χρήση signal SIGUSR2, από τον Server προς τον Commander, για
    να ενημερώσει ότι ο Server έχει δημιουργήσει semaphores για τον συγχονισμό της 
    αποστολής των πακέτων. Για κάθε πακέτο στέλνετε το μήκος του και το ίδιο το πακέτο
    arguments.
    - Σε κάθε περίπτωση γίνεται χρήση του signal SIGUSR1, από τον Commander προς τον Server,
    για να μπει σε διαδικασία διαβάσματος των arguments.
    - Ο Commander ανοίγει το fifo pipe για διάβασμα από τον Server (αφού εκτελεστούν οι 
    εντολές) και περιμένει τον Server να επιστρέψει το μήνυμα. Ανάλογα το μήνυμα, αποφασίζει 
    αν πρέπει να το εκτυπώσει ή όχι.
    - Ο Commander κλείνει τα δύο pipes από τη μεριά του.

3. Το αρχείο jobExecutorServer.c χρησιμοποείται για τη παραλαβή του μηνύματος, την 
επεξεργασία του και τη κλήση άλλων συναρτήσεων για την εκτέλεση των εντολών.
    - Η main() του αρχείου δημιουργεί το .txt αρχείο, τα δύο Queue για τα waiting και running
    processes και το fifo pipe για το γράψιμο του Server και διάβασμα του Commander.
    - Ανοίγει το pipe για το διάβασμα του Commander από τον Server.
    - Αρχικοποιεί το struct που κρατά τις πληροφορίες του Server.
    - Και περιμένει να πάρει signal από τον Commander για μεταφορά μηνύματος ή από το κλείσιμο 
    ενός process παιδιού.
    - Στη περίπτωση του signal από τον Commander, καλείται ο signal handler 
    «void jobExecutorServer()», όπου διαβάζει χρήσιμες πληροφορίες για το διάβασμα του 
    μηνύματος/πακέτων. Αν υπάρχουν παραπάνω από ένα πακέτα, δημιουργεί semaphores και με τη
    χρήση αυτών και του signal SIGUSR2, γίνεται η μεταφορά των πακέτων. Αλλιώς απλά διαβάζεται
    το μήνυμα. Ύστερα το μήνυμα σπάει σε tokens μίας λέξης και καλείται η συνάρτηση «commands()»
    για τη διαχείρηση της εντολής. Τέλος, ο Server ανοίγει το fifo pipe για γράψιμο από αυτόν
    και είτε γράφει το μήνυμα που πρέπει να σταλεί στον Commander είτε στέλνει "-1" για να δείξει
    ότι δεν υπάρχει μήνυμα μετά κλείνει το pipe.
    - Στην περίπτωση του signal SIGCHLD από τον τερματισμό ενός process παιδιού καλείται ο 
    signal_handler/συνάρτηση «void exec_commands_in_queue()» η χρήση αυτού είναι να εκτελέσει,
    αν και όσα γίνεται processes, αλλά θα υπάρξει περαιτέρω επεξήγηση στη συνέχεια. Μέσω ενός 
    active while loop και της waitpid με flag «WNOHANG» με τον τερματισμό ενός παιδιού και 
    επιστροφή του signal SIGCHLD, καλείται και η συνάρτηση remove_pid_from_run_queue() που 
    διαγράφει το pid του child process που τελείωσε από το running queue.
    - Αν σταλεί η εντολή «./jobCommander exit», ο Server κλείνει το pipe του για διάβασμα, 
    διαγράφει το .txt file, ελευθερώνει τη μνήμη για τα Triplets και διαγράφει τα Queues.
    Με το τρόπο αυτό ο Server κλείνει.

4. Η διαχείρηση των commands από τον Commander προς τον Server γίνεται με τα αρχεία:
    - ServerCommands.h: Περιέχει τον ορισμό του struct ServerInfo για το information του Server,
    τη δήλωση των συναρτήσεων για κάθε command.
    - ServerCommands.c: Η υλοποίηση σε κώδικα των παραπάνω.
    4.1. Η συνάρτηση «char* commands(char** tokenized, char* unix_command)»:
        - Ξεκαθαρίζει ποια κατηγορία command καλείται και είτε καλεί την βοηθητική της είτε 
        εκτελεί τη λειτουργία της.
        - Επιστρέφει επίσης "-1" για μη επιστροφή μηνύματος στον Commander ή το μήνυμα.
    4.2. Η συνάρτηση/signal_handler «exec_commands_in_queue(int sig)»:
        - Ελέγχει αν μπορούν να αρχίσουν την εκτέλεση τους άλλα processes και αν μπορούν εκτελεί 
        όσα γίνεται.
        - Αυτό το κάνει, παίρνοντας κάθε φορά, το πρώτο process από το waiting Queue, βάζοντας το
        στο running Queue, φτιάχνοντας το queue position για κάθε process στο waiting Queue και
        εκτελώντας το χρησιμοποιόντας μία fork() και μία execvp().
    4.3. Η συνάρτηση «Triplet* issueJob(char* job)»: 
        - Χρησιμοποιείται για το command «issueJob».
        - Δημιουργεί ένα Triplet για το job και το βάζει στο queue.
        - Καλεί την συνάρτηση «exec_commands_in_queue()» για να εκτελέσει, αν γίνεται, όσα process
        γίνεται.
        - Επιστρέφει το Triplet.
    4.4. Η συνάρτηση «char* stop_job(char** tokenized)»:
        - Χρησιμοποιείται για το command «stop jobID».
        - Ελέγχει αν η εργασία με το jobID τρέχει. Αν ναι τη σκοτώνει και την βγάζει από το running 
        Queue, αλλιώς αν είναι μέσα στο waiting Queue, την αφαιρεί από αυτό.
        - Επιστρέφει μήνυμα ειδοποίησης του τι έκανε.
    4.5. Η συνάρτηση «char* poll(char** tokenized)»:
        - Χρησιμοποιείται για το command «poll running/queued».
        - Βρίσκει ποια από τις δύο ουρές θέλουμε, βρίσκει το συνολικό μέγεθος των Triplet υπό τη
        μορφή string και κατασκευάζει ένα μεγάλο μήνυμα (string) με όλα τα Triplets στο Queue.
        - Επιστρέφει το μήνυμα.

5. Bash Script «multijob.sh»:
    - Για κάθε .txt αρχείο που δέχεται ως argument φτιάχνει ένα νέο ίδιο temp.txt αρχείο (που 
    ύστερα διαγράφεται), όμως στην αρχή κάθε γραμμής του νέου .txt βάζει και το string 
    «./jobCommander issueJob ».
    - Τρέχει κάθε εντολή του αρχείου .txt

6. Bash Script «allJobsStop.sh»:
    - Για το waiting Queue, τρέχει και βάζει το output του command «/jobCommander poll queued» σε 
    μία μεταβλητή.
    - Από αυτή τη μεταβλητή βρίσκει, αν υπάρχει, το τελευταίο jobID και μετά καλεί την εντολή
    «./jobCommander stop "$temp_jobID"» για κάθε ID μικρότερο του τελευταίου, ώστε να τερματίσει 
    κάθε jobID.
    - Κάνει το ίδιο και για το running Queue.

7. Ιδιαιτερότητες σε test:
    - test_jobExecutor_6.sh: Η υλοποίηση μου, με τον τερματισμό κάθε child process μέσω της εντολής
    stop jobID, στέλνει signal SIGCHLD. Έτσι κατευθείαν μετά την εντολή stop, αν υπάρχει process στο
    waiting Queue, γίνεται execute μία εντολή από αυτό. Για αυτό το λόγο, τα αποτέσματα μου έχουν 
    κάποιες μικρές διαφορές σε αυτό το test, σχετικά με τα υποδεικνυόμενα.
    - test_sh_scripts_1.sh: Ενώ όλες οι εντολές φαίνονται να εισέρχονται στο waiting Queue, 
    εκτελούνται μόνο μέχρι ένα σημείο, δεν μπόρεσα να βρω τον λόγο.