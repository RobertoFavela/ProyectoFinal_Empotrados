function actualizarEstado() {
  fetch("/estado")
    .then(response => {
      if (!response.ok) throw new Error("Fallo en la red");
      return response.json();
    })
    .then(data => {
      // Puerta
      const estadoPuerta = document.getElementById("estadoPuerta");
      estadoPuerta.textContent = data.puerta.toUpperCase();
      estadoPuerta.style.color = data.puerta === "abierta" ? "green" : "red";

      // Lugares
      const lugares = document.querySelectorAll(".lugar");
      lugares[0].textContent = `Lugar 1: ${data.lugar1?.toUpperCase() ?? "DESCONOCIDO"}`;
      lugares[1].textContent = `Lugar 2: ${data.lugar2?.toUpperCase() ?? "DESCONOCIDO"}`;
      lugares[2].textContent = `Lugar 3: ${data.lugar3?.toUpperCase() ?? "DESCONOCIDO"}`;

      // Luz
      const estadoLuz = document.getElementById("estadoLuz");
      estadoLuz.textContent = data.luz.toUpperCase();
      estadoLuz.style.color = data.luz === "encendida" ? "gold" : "gray";

      // Valor fotoresistencia
      const fotoValor = document.getElementById("fotoValor");
      if (fotoValor) {
        fotoValor.textContent = data.fotoValor;
      }

      // Umbral actual
      const umbralInput = document.getElementById("umbralInput");
      if (umbralInput && document.activeElement !== umbralInput) {
        umbralInput.value = data.umbral;
      }

      // Bloqueo entrada
      const checkbox = document.getElementById("bloqueoEntrada");
      if (checkbox) {
        checkbox.checked = data.bloqueado;
      }
    })
    .catch(error => console.error("Error al obtener estado:", error));
}

function actualizarUmbral() {
  const nuevoUmbral = document.getElementById("umbralInput").value;
  fetch("/umbral", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded"
    },
    body: "valor=" + encodeURIComponent(nuevoUmbral)
  })
    .then(response => {
      if (!response.ok) throw new Error("Fallo al actualizar umbral");
      return response.text();
    })
    .then(msg => console.log("Umbral actualizado:", msg))
    .catch(error => console.error("Error al actualizar umbral:", error));
}

function cambiarBloqueoEntrada(event) {
  const activo = event.target.checked;
  fetch("/bloqueo", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded"
    },
    body: "activo=" + encodeURIComponent(activo)
  })
    .then(response => {
      if (!response.ok) throw new Error("Error al actualizar bloqueo");
      return response.text();
    })
    .then(msg => console.log("Bloqueo actualizado:", msg))
    .catch(error => console.error("Error al cambiar bloqueo:", error));
}

document.addEventListener("DOMContentLoaded", () => {
  const checkbox = document.getElementById("bloqueoEntrada");
  if (checkbox) {
    checkbox.addEventListener("change", cambiarBloqueoEntrada);
  }
});

actualizarEstado();
setInterval(actualizarEstado, 1000);
