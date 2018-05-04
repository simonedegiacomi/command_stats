# Lifecycle

Prima volta che invoco run
1. Controlla se può scrivere il file di log, e lo crea se non esiste
2. Controllo se il logger è in esecuzione, altrimenti lo avvio
3. Parsing dell'input
4. Esecuzione
5. Chiedo al logger di poter scrivere sulla pipe e attende (wait, si sveglia con timeout o segnale)
6. Il logger da il via (lo sveglia) attraverso il segnale
7. Run apre la pipe in scrittura, scrive e chiude



