// SmartSuggest Content Script
// Active on Gmail, Outlook, LinkedIn, and general web textareas

let activeField = null;
let debounceTimer = null;
let currentIssues = [];
let lastCorrected = '';
let badgeEl = null;
let popoverEl = null;

function init() {
  createFloatingBadge();
  attachInputListeners();
}

function createFloatingBadge() {
  if (document.getElementById('smartsuggest-badge')) return;

  badgeEl = document.createElement('div');
  badgeEl.id = 'smartsuggest-badge';
  badgeEl.className = 'smartsuggest-badge ready';
  badgeEl.innerHTML = `
    <span class="ss-dot"></span>
    <span class="ss-label" id="ssBadgeText">SmartSuggest (Ready)</span>
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
      badgeEl.classList.remove('hidden');
      const text = getTextFromField(activeField);
      if (text.trim().length >= 3) {
        triggerCheck(text);
      } else {
        updateBadge(0, 'ready');
      }
    }
  });

  document.addEventListener('input', (e) => {
    if (isEditableField(e.target)) {
      activeField = e.target;
      const text = getTextFromField(activeField);

      if (text.trim().length < 3) {
        updateBadge(0, 'ready');
        return;
      }

      updateBadge(0, 'checking');
      clearTimeout(debounceTimer);
      debounceTimer = setTimeout(() => {
        triggerCheck(text);
      }, 350);
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

function triggerCheck(text) {
  if (!chrome.runtime?.id) {
    console.warn('SmartSuggest: Extension context updated. Please refresh tab.');
    return;
  }

  chrome.runtime.sendMessage(
    { type: 'CHECK_TEXT', mode: 'grammar', text: text.trim() },
    (response) => {
      if (chrome.runtime.lastError || !response || !response.success) {
        updateBadge(0, 'offline');
      } else {
        currentIssues = response.data.issues || [];
        lastCorrected = response.data.corrected || text;
        updateBadge(currentIssues.length, currentIssues.length === 0 ? 'clean' : 'issues');
        renderPopoverContent(currentIssues);
      }
    }
  );
}

function updateBadge(issueCount, state = 'ready') {
  if (!badgeEl) return;
  const label = badgeEl.querySelector('#ssBadgeText');

  badgeEl.className = 'smartsuggest-badge';

  if (state === 'offline') {
    badgeEl.classList.add('error');
    label.textContent = 'SmartSuggest (Offline)';
  } else if (state === 'checking') {
    badgeEl.classList.add('checking');
    label.textContent = 'Checking...';
  } else if (state === 'ready') {
    badgeEl.classList.add('ready');
    label.textContent = 'SmartSuggest (Ready)';
  } else if (state === 'clean') {
    badgeEl.classList.add('clean');
    label.textContent = '✓ Clean text';
  } else if (state === 'issues') {
    badgeEl.classList.add('issues');
    label.textContent = `⚡ ${issueCount} Issue${issueCount > 1 ? 's' : ''}`;
  }
}

function togglePopover() {
  if (popoverEl.classList.contains('hidden')) {
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
      <p class="ss-clean-msg">No spelling or grammar issues found in this text.</p>
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

  issues.slice(0, 6).forEach((iss) => {
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
      ✦ Apply Corrections to Field
    </button>
  `;

  popoverEl.innerHTML = html;

  const btn = popoverEl.querySelector('#ssApplyAllBtn');
  if (btn) {
    btn.addEventListener('click', () => {
      if (activeField && lastCorrected) {
        setTextInField(activeField, lastCorrected);
        popoverEl.classList.add('hidden');
        updateBadge(0, 'clean');
      }
    });
  }
}

init();
