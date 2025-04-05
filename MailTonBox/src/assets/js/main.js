window.onload = function() {
    const iframe = document.getElementById("myIframe");
    const modal = document.getElementById("loading");
    const modal2 = document.getElementById("myModal2");
    const retryButton = document.getElementById("retryButton");
    let attempts = 0;
    const maxAttempts = 3;
    const timeoutInterval = 5000; // 5 secondi tra i tentativi
    let iframeLoadedSuccessfully = false;
    let checkTimeout;

    // Mostriamo immediatamente il modal
    function showModal() {
    modal.style.display = "flex";
    }
    function showModal2() {
    modal2.style.display = "flex";
    }
    function hideModal() {
    modal.style.display = "none";
    modal2.style.display = "none";
    }

    // Questa funzione verifica se l'iframe contiene l'elemento "validContent"
    // che deve essere presente solo se la pagina è stata caricata correttamente.
    function checkIframeValid() {
        try {
            const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
            if (iframeDoc && iframeDoc.getElementById("=^.^=")) {
                return true;
            }
        } catch (e) {
            console.log("Errore durante il controllo del contenuto dell'iframe:", e);
        }
        return false;
    }

    // Ricarica l'iframe e pianifica un nuovo controllo
    function reloadIframe() {
        if (!iframeLoadedSuccessfully && attempts < maxAttempts) {
            attempts++;
            console.log(`Tentativo ${attempts}/${maxAttempts}`);
            // Forza il reload aggiungendo un timestamp all'URL per evitare la cache
            iframe.src = iframe.src.split("?")[0] + "?t=" + new Date().getTime();
            checkTimeout = setTimeout(checkAndRetry, timeoutInterval);
        } else if (!iframeLoadedSuccessfully) {
            console.log("Superati i tentativi massimi, il modal resta visibile.");
            hideModal();
            showModal2();
        }
    }

    // Verifica se il caricamento è andato a buon fine, altrimenti ricarica
    function checkAndRetry() {
        if (checkIframeValid()) {
            console.log("Iframe caricato correttamente.");
            iframeLoadedSuccessfully = true;
            hideModal();
            clearTimeout(checkTimeout);
        } else {
            reloadIframe();
        }
    }

    // Se l'utente clicca il pulsante di ricarica, resettare i tentativi e riprovare
    retryButton.onclick = function() {
        attempts = 0;
        iframeLoadedSuccessfully = false;
        
        hideModal();
        
        showModal();
        reloadIframe();
    };

    // Evento onload dell'iframe: esegui il controllo immediatamente dopo il caricamento
    iframe.onload = function() {
        console.log("Evento onload dell'iframe.");
        if (checkIframeValid()) {
            console.log("Valid content detected in the hypiframe.");
            iframeLoadedSuccessfully = true;
            hideModal();
            clearTimeout(checkTimeout);
        } else {
            console.log("Not valid content, the Modal remains.");
        }
    };

    // Mostra immediatamente il modal e avvia il primo controllo
    showModal();
    checkTimeout = setTimeout(checkAndRetry, timeoutInterval);
};