// Background Service Worker for SmartSuggest
// Proxies API requests from HTTPS pages (Gmail, Outlook) to http://localhost:8080 to bypass Mixed Content restrictions

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.type === 'CHECK_TEXT') {
    const mode = request.mode || 'grammar';
    const text = request.text || '';
    const url = `http://localhost:8080/suggest?mode=${mode}&q=${encodeURIComponent(text)}`;

    fetch(url)
      .then(async (res) => {
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const data = await res.json();
        sendResponse({ success: true, data });
      })
      .catch((err) => {
        sendResponse({ success: false, error: err.message });
      });

    return true; // Keep message port open for async response
  }
});
