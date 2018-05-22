# Problemi
- Gestire l'accesso di 2 run alla named-pipe di input del logger
    + Risolto usando i messaggi. I run chiedono il permesso al logger e illogger abilita un run alla volta
- Quando viene settata la path? più run specificano diverse path di log
    + Abbiamo deciso che può essere specificata una path diversa ad ogni invocazione del run. Quando il run chiede il permesso di usare la pipe al logger (attraverso un messaggio), run specifica anche la path.
- Gestire cd

# Domande
- **Quali statistiche recuperare e da dove**;
- Funzionamento delle message queue (creazione del file oppure no);
- Procedura per fare un demone;