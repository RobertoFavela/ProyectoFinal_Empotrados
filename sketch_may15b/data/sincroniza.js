function actualizarEstadoPuerta() {
    fetch("/estado")
      .then(response => response.text())
      .then(estado => {
        const texto = estado.toUpperCase();
        document.getElementById("estadoPuerta").textContent = texto;
        if (estado === "abierta") {
          document.getElementById("estadoPuerta").style.color = "green";
        } else {
          document.getElementById("estadoPuerta").style.color = "red";
        }
      });
  }
  
  // Actualiza el estado
  setInterval(actualizarEstadoPuerta, 1000);
  
  // llamada inicial
  actualizarEstadoPuerta();
  