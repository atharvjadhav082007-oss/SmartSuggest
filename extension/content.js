// SmartSuggest In-Page Content Script
// Active on Gmail, Outlook, and web compose areas

const API_ENDPOINT = 'http://localhost:8080/suggest';
let activeField = null;
let debounceTimer = null;
let currentIssues = [];
let lastCorrected = '';
let badgeEl = null;
let popoverEl = null;

// Initialize on page load
function init() {
  createFloatingBadge();
  attachInputListeners();
}

// Create floating pill in bottom right of page
function createFloatingBadge() {
  if (document.getElementById('smartsuggest-badge')) return;

  badgeEl = document.createElement('div');
  badgeEl.id = 'smartsuggest-badge';
  badgeEl.className = 'smartsuggest-badge hidden';
  badgeEl.innerHTML = `
    <span class="ss-dot"></span>
    <span class="ss-label" id="ssBadgeText">SmartSuggest</span>
  `;
  document.body.appendChild(badgeEl);

  popoverEl = document.createElement('div');
  popoverEl.id = 'smartsuggest-popover';
  popoverEl.className = 'smartsuggest-popover hidden';
  document.body.appendChild(popoverEl);

  badgeEl.addEventListener('click', (e) => {
    e.stopPropagation();
    togglePopover();
  });

  document.addEventListener('click', (e) => {
    if (!popoverEl.contains(e.target) && !badgeEl.contains(e.target)) {
      popoverEl.classList.add('hidden');
    }
  });
}

function attachInputListeners() {
  document.addEventListener('focusin', (e) => {
    if (isEditableField(e.target)) {
      activeField = e.target;
      positionBadgeNear(activeField);
      badgeEl.classList.remove('hidden');
      triggerCheck(getTextFromField(activeField));
    }
  });

  document.addEventListener('input', (e) => {
    if (isEditableField(e.target)) {
      activeField = e.target;
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(() => {
        triggerCheck(getTextFromField(activeField));
      }, 400);
    }
  });
}

function isEditableField(el) {
  if (!el) return false;
  const tag = el.tagName ? el.tagName.toLowerCase() : '';
  const isInput = tag === 'textarea' || (tag === 'input' && el.type === 'text');
  const isContentEditable = el.isContentEditable || el.getAttribute('contenteditable') === 'true' || el.getAttribute('role') === 'textbox';
  return isInput || isContentEditable;
}

function getTextFromField(el) {
  if (!el) return '';
  if (el.tagName && el.tagName.toLowerCase() === 'textarea') {
    return el.value;
  }
  return el.innerText || el.textContent || '';
}

function setTextInField(el, newText) {
  if (!el) return;
  if (el.tagName && el.tagName.toLowerCase() === 'textarea') {
    el.value = newText;
    el.dispatchEvent(new Event('input', { bubbles: true }));
  } else {
    el.innerText = newText;
    el.dispatchEvent(new Event('input', { bubbles: true }));
  }
}

function positionBadgeNear(field) {
  const rect = field.getBoundingClientRect();
  badgeEl.style.top = `${Math.min(window.innerHeight - 50, rect.bottom - 40)}px`;
  badgeEl.style.left = `${Math.min(window.innerWidth - 160, rect.right - 140)}px`;
}

async function triggerCheck(text) {
  if (!text || text.trim().length < 3) {
    updateBadge(0, false, 'ready');
    return;
  }

  // Use background service worker to fetch from http://localhost:8080
  // This bypasses HTTPS Mixed Content blocks on Gmail / Outlook
  if (chrome.runtime && chrome.runtime.sendMessage) {
    chrome.runtime.sendMessage(
      { type: 'CHECK_TEXT', mode: 'grammar', text: text.trim() },
      (response) => {
        if (chrome.runtime.lastError || !response || !response.success) {
          updateBadge(0, true);
        } else {
          currentIssues = response.data.issues || [];
          lastCorrected = response.data.corrected || text;
          updateBadge(currentIssues.length, false);
          renderPopoverContent(currentIssues);
        }
      }
    );
  } else {
    // Fallback direct fetch
    try {
      const res = await fetch(`${API_ENDPOINT}?mode=grammar&q=${encodeURIComponent(text.trim())}`);
      if (!res.ok) throw new Error();
      const data = await res.json();
      currentIssues = data.issues || [];
      lastCorrected = data.corrected || text;
      updateBadge(currentIssues.length, false);
      renderPopoverContent(currentIssues);
    } catch (err) {
      updateBadge(0, true);
    }
  }
}

function updateBadge(issueCount, isError, state = '') {
  const dot = badgeEl.querySelector('.ss-dot');
  const label = badgeEl.querySelector('#ssBadgeText');

  badgeEl.className = 'smartsuggest-badge';

  if (isError) {
    badgeEl.classList.add('error');
    label.textContent = 'SmartSuggest (Offline)';
  } else if (state === 'ready') {
    badgeEl.classList.add('ready');
    label.textContent = 'SmartSuggest (Ready)';
  } else if (issueCount === 0) {
    badgeEl.classList.add('clean');
    label.textContent = '✓ Clean text';
  } else {
    badgeEl.classList.add('issues');
    label.textContent = `⚡ ${issueCount} Issue${issueCount > 1 ? 's' : ''}`;
  }
}

function togglePopover() {
  if (popoverEl.classList.contains('hidden')) {
    const badgeRect = badgeEl.getBoundingClientRect();
    popoverEl.style.top = `${badgeRect.top - 230}px`;
    popoverEl.style.left = `${Math.max(10, badgeRect.left - 120)}px`;
    popoverEl.classList.remove('hidden');
  } else {
    popoverEl.classList.add('hidden');
  }
}

function renderPopoverContent(issues) {
  if (!issues || issues.length === 0) {
    popoverEl.innerHTML = `
      <div class="ss-pop-header">
        <strong>SmartSuggest Assistant</strong>
        <span class="ss-clean-tag">✓ Clean</span>
      </div>
      <p class="ss-clean-msg">No spelling or grammar issues detected in active email/box.</p>
    `;
    return;
  }

  let html = `
    <div class="ss-pop-header">
      <strong>SmartSuggest Proofreader</strong>
      <span class="ss-issues-tag">${issues.length} Issue${issues.length > 1 ? 's' : ''}</span>
    </div>
    <div class="ss-pop-list">
  `;

  issues.slice(0, 5).forEach((iss) => {
    html += `
      <div class="ss-pop-item ${iss.severity}">
        <div class="ss-pop-diff">
          ${iss.original ? `<span class="ss-orig">${iss.original}</span> ➔ ` : ''}
          <span class="ss-sugg">${iss.suggestion || 'Remove'}</span>
        </div>
        <div class="ss-pop-msg">${iss.message}</div>
      </div>
    `;
  });

  html += `
    </div>
    <button type="button" class="ss-apply-btn" id="ssApplyAllBtn">
      ✦ Apply Corrections to Email
    </button>
  `;

  popoverEl.innerHTML = html;

  const btn = popoverEl.querySelector('#ssApplyAllBtn');
  if (btn) {
    btn.addEventListener('click', () => {
      if (activeField && lastCorrected) {
        setTextInField(activeField, lastCorrected);
        popoverEl.classList.add('hidden');
        updateBadge(0, false);
      }
    });
  }
}

// Start
init();
