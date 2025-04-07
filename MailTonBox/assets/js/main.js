let deferredPrompt;

function isInStandaloneMode() {
  return window.matchMedia('(display-mode: standalone)').matches || window.navigator.standalone === true;
}

const urlParams =  new URLSearchParams(window.location.search);

if (urlParams.has('install') && !isInStandaloneMode() && localStorage.getItem("pwaInstall") !== "true") {
    const modal1 = document.getElementById("loading");
    if (modal1) modal1.style.display = "none";
    const modal = document.getElementById("install");
    if (modal) modal.style.display = "flex";
}

window.addEventListener('beforeinstallprompt', (e) => {
    e.preventDefault(); // Blocca il comportamento automatico
    deferredPrompt = e;

    const installBtn = document.getElementById('install-btn');
    installBtn.style.display = 'inline-block';

    deferredPrompt.prompt();

    // (Facoltativo) Gestisci la scelta dell’utente
    deferredPrompt.userChoice.then((choiceResult) => {
        console.log('Utente ha scelto:', choiceResult.outcome);
        deferredPrompt = null;
    });

    // 👇 Il bottone resta cliccabile per dopo
    installBtn.addEventListener('click', () => {
        if (deferredPrompt) {
            deferredPrompt.prompt();
            deferredPrompt.userChoice.then((choiceResult) => {
              console.log("Utente ha scelto:", choiceResult.outcome);
              if (choiceResult.outcome === 'dismissed') {
                  // Salva che ha rifiutato
                  localStorage.setItem("pwaInstall", "false");
                  const url = new URL(window.location);
                  url.searchParams.add('install');
                  window.history.replaceState({}, '', url);
                  const modal = document.getElementById("install");
                  modal.style.display = "flex";
              }
              if (choiceResult.outcome === 'accepted') {
                localStorage.setItem("pwaInstall", "true");
                  const modal = document.getElementById("install");
                  if (modal) modal.style.display = "none";
                  const url = new URL(window.location);
                  url.searchParams.delete('install');
                  window.history.replaceState({}, '', url);
              }
            });
        }
    });
});

window.onload = async function () {
  const modal = document.getElementById("loading");
  const modal2 = document.getElementById("myModal2");
  const retryButton = document.getElementById("retryButton");
  const showIstruction = document.getElementById("showIstruction");

  function showModal() {
    modal.style.display = "flex";
  }

  function showModal2() {
    modal2.style.display = "flex";
  }

  function hideModals() {
    modal.style.display = "none";
    modal2.style.display = "none";
  }

  showModal();
  
  showIstruction.onclick = function () {
    clearTimeout(fallbackTimeout);
    hideModals();
    showModal2();
  };

  const fallbackTimeout = setTimeout(() => {
    hideModals();
    showModal2();
  }, 5000); // 5s
  
  retryButton.onclick = function () {
    clearTimeout(fallbackTimeout);
    hideModals();
    showModal();

    // Restart the fallback timeout
    fallbackTimeout = setTimeout(() => {
      hideModals();
      showModal2();
    }, 5000); // 5s
  };

  function openFullscreen() {
    const elem = document.documentElement;
    if (elem.requestFullscreen) {
      elem.requestFullscreen();
    } else if (elem.mozRequestFullScreen) { 
      elem.mozRequestFullScreen();
    } else if (elem.webkitRequestFullscreen) { 
      elem.webkitRequestFullscreen();
    } else if (elem.msRequestFullscreen) { 
      elem.msRequestFullscreen();
    }
  }
  
  openFullscreen(); 
};