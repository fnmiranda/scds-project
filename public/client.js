// public/client.js
(() => {
  const sock = io();
  // o servidor envia objetos { timestamp, events, duration, intensity }
  sock.on('reading', (msg) => {
    // garante campos básicos
    const item = {
      timestamp: msg.timestamp || new Date().toISOString().replace('T',' ').slice(0,19),
      events: Number(msg.events || 0),
      duration: Number(msg.duration || 1), // em minutos
      intensity: Number(msg.intensity || 0)
    };
    if (window.appendArduinoItem) window.appendArduinoItem(item);
  });

  // opcional: receber histórico inicial
  sock.on('bootstrap', (arr) => {
    if (window.replaceAllData) window.replaceAllData(arr);
  });
})();
