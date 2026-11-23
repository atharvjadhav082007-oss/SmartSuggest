const statusBadge = document.getElementById('statusBadge');
const statusText = document.getElementById('statusText');
const quickInput = document.getElementById('quickInput');
const checkBtn = document.getElementById('checkBtn');
const copyBtn = document.getElementById('copyBtn');
const resultsBox = document.getElementById('resultsBox');
const resultsMeta = document.getElementById('resultsMeta');
const issuesList = document.getElementById('issuesList');

let lastCorrected = '';

async function queryServer(mode, text) {
  const tryHost = (host) => {
    return fetch(`http://${host}:8080/suggest?mode=${mode}&q=${encodeURIComponent(text)}`, {
      method: 'GET',
      headers: { 'Accept': 'application/json' }
    }).then(async res => {
      if (!res.ok) throw new Error();
      return await res.json();
    });
  };

  return tryHost('127.0.0.1').catch(() => tryHost('localhost'));
}

async function checkServerStatus() {
  try {
    await queryServer('autocomplete', 'test');
    statusBadge.className = 'status-indicator online';
    statusText.textContent = 'C++ Live (8080)';
  } catch (err) {
    statusBadge.className = 'status-indicator offline';
    statusText.textContent = 'Server Offline';
  }
}

async function proofreadText() {
  const text = quickInput.value.trim();
  if (!text) return;

  checkBtn.disabled = true;
  checkBtn.textContent = 'Checking...';

  try {
    const data = await queryServer('grammar', text);

    lastCorrected = data.corrected || text;
    copyBtn.disabled = false;
    resultsBox.style.display = 'flex';

    const issues = data.issues || [];
    resultsMeta.textContent = `${issues.length} issue${issues.length === 1 ? '' : 's'} detected:`;

    if (issues.length === 0) {
      issuesList.innerHTML = `
        <div style="color: #15803d; font-size: 0.8rem; font-weight: 700; padding: 6px 0;">
          ✓ No grammar or spelling issues found!
        </div>
      `;
    } else {
      issuesList.innerHTML = issues.map(iss => `
        <div class="issue-item ${iss.severity}">
          <div class="issue-diff">
            ${iss.original ? `<span class="issue-original">${iss.original}</span> ➔ ` : ''}
            <span class="issue-fix">${iss.suggestion || 'Remove'}</span>
          </div>
          <div class="issue-msg">${iss.message}</div>
        </div>
      `).join('');
    }
  } catch (err) {
    resultsBox.style.display = 'flex';
    resultsMeta.textContent = 'Error connecting to engine';
    issuesList.innerHTML = `
      <div style="color: #dc2626; font-size: 0.75rem;">
        Ensure <code>app.exe --server</code> is running on port 8080.
      </div>
    `;
  } finally {
    checkBtn.disabled = false;
    checkBtn.textContent = '⚡ Proofread';
  }
}

checkBtn.addEventListener('click', proofreadText);

copyBtn.addEventListener('click', () => {
  if (lastCorrected) {
    navigator.clipboard.writeText(lastCorrected).then(() => {
      copyBtn.textContent = '✓ Copied!';
      setTimeout(() => {
        copyBtn.textContent = '📋 Copy Fixed';
      }, 1500);
    });
  }
});

checkServerStatus();
