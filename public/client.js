// public/client.js
(() => {
  const sock = io();

  // Converte msg (string ou objeto) para o shape esperado pelo front
  function normalize(msg) {
    // se chegar como string, tenta JSON.parse
    if (typeof msg === 'string') {
      try { msg = JSON.parse(msg); } catch { msg = {}; }
    }

    const ts = msg.timestamp || new Date().toISOString().replace('T',' ').slice(0,19);

    // duration em MINUTOS: se vier duration_ms, converte; senão usa duration
    const durationMin =
      (msg.duration_ms != null) ? (Number(msg.duration_ms) / 60000) :
      (msg.duration     != null) ?  Number(msg.duration) : 1;

    // intensidade: aceita peak_intensity como fallback
    const intensity =
      (msg.intensity       != null) ? Number(msg.intensity) :
      (msg.peak_intensity  != null) ? Number(msg.peak_intensity) : 0;

    return {
      timestamp: ts,
      events: Number(msg.events || 0),
      duration: durationMin,   // MINUTOS
      intensity
    };
  }

  function handleItem(msg) {
    const item = normalize(msg);
    if (window.appendArduinoItem) window.appendArduinoItem(item);
  }

  // Ouça ambos (compatibilidade)
  sock.on('data', handleItem);      // recomendado (server: io.emit('data', obj))
  sock.on('reading', handleItem);   // compat com seu evento antigo

  // Histórico inicial (opcional)
  sock.on('bootstrap', (arr = []) => {
    const norm = arr.map(normalize);
    if (window.replaceAllData) window.replaceAllData(norm);
  });

  // Debug opcional
  sock.on('connect',    () => console.log('[socket] conectado'));
  sock.on('disconnect', () => console.log('[socket] desconectado'));
})();
