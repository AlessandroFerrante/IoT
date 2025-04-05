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