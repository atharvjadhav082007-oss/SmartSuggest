// Background Service Worker for SmartSuggest
// Proxies API requests from HTTPS pages (Gmail, Outlook) to local C++ engine
// Handles both 127.0.0.1 (IPv4) and localhost seamlessly

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.type === 'CHECK_TEXT') {
    const mode = request.mode || 'grammar';
    const text = request.text || '';

    const tryHost = (host) => {
      return fetch(`http://${host}:8080/suggest?mode=${mode}&q=${encodeURIComponent(text)}`, {
        method: 'GET',
        headers: { 'Accept': 'application/json' }
      }).then(async (res) => {
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        return await res.json();
      });
    };

    tryHost('127.0.0.1')
      .catch(() => tryHost('localhost'))
      .then((data) => sendResponse({ success: true, data }))
      .catch((err) => sendResponse({ success: false, error: err.message }));

    return true; // Async response
  }

  if (request.type === 'PING') {
    const checkPing = (host) => fetch(`http://${host}:8080/suggest?mode=autocomplete&q=test`);
    checkPing('127.0.0.1')
      .catch(() => checkPing('localhost'))
      .then(res => sendResponse({ online: res.ok }))
      .catch(() => sendResponse({ online: false }));
    return true;
  }
});
