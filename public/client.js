// public/client.js
(() => {
  const sock = io();

  // Converte msg (string ou objeto) para o shape esperado pelo front
function normalize(msg){
  if (typeof msg === 'string') { try { msg = JSON.parse(msg); } catch { msg = {}; } }

  const ts = msg.timestamp || new Date().toISOString().replace('T',' ').slice(0,19);

  const durationMin =
    (msg.duration_ms != null) ? (Number(msg.duration_ms) / 60000) :
    (msg.duration     != null) ?  Number(msg.duration) : 0;

  const intensity =
    (msg.intensity      != null) ? Number(msg.intensity) :
    (msg.peak_intensity != null) ? Number(msg.peak_intensity) : 0;

  return {
    timestamp: ts,
    events: Number(msg.events || 0),     // eventos do BUCKET
    duration: durationMin,               // minutos (só métrica de janela)
    intensity,
    dt_ms: Number(msg.dt_ms || 0),       // Δt real entre tiros
    t_ms:  Number(msg.t_ms  || 0)        // relógio monotônico do device
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
