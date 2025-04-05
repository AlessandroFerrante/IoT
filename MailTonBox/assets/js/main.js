window.onload = async function () {
  const modal = document.getElementById("loading");
  const modal2 = document.getElementById("myModal2");
  const retryButton = document.getElementById("retryButton");
  const openBtn = document.getElementById("openBtn");

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

  const fallbackTimeout = setTimeout(() => {
    hideModals();
    showModal2();
  }, 5000); // 5s

  retryButton.onclick = function () {
    clearTimeout(fallbackTimeout);
    hideModals();
    showModal();
  };

  openBtn.onclick = function () {
    clearTimeout(fallbackTimeout);
    window.open("http://192.168.4.1", "_blank");
  };

  function openFullscreen() {
    const elem = document.documentElement;
    if (elem.requestFullscreen) {
      elem.requestFullscreen();
    } else if (elem.mozRequestFullScreen) { // Firefox
      elem.mozRequestFullScreen();
    } else if (elem.webkitRequestFullscreen) { // Chrome, Safari and Opera
      elem.webkitRequestFullscreen();
    } else if (elem.msRequestFullscreen) { // IE/Edge
      elem.msRequestFullscreen();
    }
  }
  
  openFullscreen();  // Chiamato quando necessario
};