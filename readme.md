
# 🎫 Boletomaster

**Boletomaster** es una plataforma integral para la gestión de eventos musicales, venta de boletos, lanzamientos discográficos y comercialización de mercancía oficial. El sistema conecta a artistas, organizadores y fans en un solo ecosistema digital.

---

## 🚀 Características Principales

- **Gestión de Eventos:** Creación y visualización de eventos con detalles de locación y fechas.
- **Sistema de Boletos:** Venta de tickets con integración de pagos.
- **Tienda de Mercancía (Merch):** Catálogo de productos con gestión de carrito y stock.
- **Perfiles de Artista:** Secciones dedicadas para biografía, lanzamientos y galería multimedia.
- **Roles de Usuario:** Soporte para Clientes, Artistas, Organizadores y Administradores.
- **Pagos Seguros:** Integración con la API de **PayPal** para transacciones en tiempo real.

---

## 🛠️ Stack Tecnológico

### Backend
- **Node.js** & **Express** (Servidor y API REST)
- **MongoDB** & **Mongoose** (Base de datos NoSQL y Modelado)
- **JWT (JSON Web Tokens)** (Autenticación y Seguridad)
- **Bcryptjs** (Cifrado de contraseñas)
- **Dotenv** (Manejo de variables de entorno)

### Frontend
- **Vanilla JavaScript** (Arquitectura basada en componentes nativos)
- **CSS3 / HTML5** (Diseño responsivo)
- **CryptoJS** (Seguridad en el manejo de datos en el cliente)

---

## 📂 Estructura del Proyecto

El proyecto sigue un patrón de diseño **DAO (Data Access Object)** para separar la lógica de la base de datos de los controladores:

```text
├── backend/
│   ├── config/          # Configuración de DB y JWT
│   ├── controllers/     # Lógica de las rutas (Artista, Evento, Orden, etc.)
│   ├── dao/             # Acceso directo a MongoDB
│   ├── models/          # Esquemas de Mongoose (Boleto, Usuario, Item, etc.)
│   ├── middleware/      # Verificación de autenticación (Auth)
│   └── server.js        # Punto de entrada del API
├── src/
│   ├── components/      # Web Components reutilizables
│   ├── pages/           # Vistas principales del sitio
│   └── utils/           # Helpers (Carrito, Formateadores, PayPal SDK)
└── index.js             # Servidor de archivos estáticos

```

---

## 🔧 Instalación y Configuración

### 1. Clonar el repositorio

```bash
git clone <url-del-repo>
cd luismario53-boletomaster

```

### 2. Configurar el Backend

Navega a la carpeta `backend/` e instala las dependencias:

```bash
cd backend
npm install

```

Crea un archivo `variable.env` en la carpeta `backend/` con las siguientes claves:

```env
PORT=5000
URL_MONGO=mongodb://127.0.0.1:27017/boletomaster
JWT_SECRET=tu_clave_secreta
PAYPAL_CLIENT_ID=tu_id_de_cliente
PAYPAL_SECRET=tu_secreto_de_paypal
PAYPAL_BASE_URL=[https://api-m.sandbox.paypal.com](https://api-m.sandbox.paypal.com)

```

### 3. Configurar el Frontend

Regresa a la raíz e instala las dependencias del servidor de archivos:

```bash
cd ..
npm install

```

---

## 🚦 Ejecución

1. **Iniciar el API (Backend):**
```bash
cd backend
npm start

```


2. **Iniciar la aplicación (Frontend):**
En una nueva terminal, desde la raíz:
```bash
npm start

```


La aplicación estará disponible en `http://localhost:3000`.

---

## 🛡️ Seguridad y Roles

El sistema utiliza middlewares de autenticación para proteger rutas sensibles:

* **Público:** Ver eventos y perfiles de artistas.
* **Cliente:** Comprar boletos y productos.
* **Artista/Organizador:** Gestión de contenido y stock.

---

**Desarrollado como parte del proyecto Boletomaster.**

```