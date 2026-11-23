const API_URL = 'http://localhost:8080/suggest';

const statusBadge = document.getElementById('statusBadge');
const statusText = document.getElementById('statusText');
const quickInput = document.getElementById('quickInput');
const checkBtn = document.getElementById('checkBtn');
const copyBtn = document.getElementById('copyBtn');
const resultsBox = document.getElementById('resultsBox');
const resultsMeta = document.getElementById('resultsMeta');
const issuesList = document.getElementById('issuesList');

let lastCorrected = '';

// Check C++ Backend Connectivity
async function checkServerStatus() {
  try {
    const res = await fetch(`${API_URL}?mode=autocomplete&q=test`, { method: 'GET' });
    if (res.ok) {
      statusBadge.className = 'status-indicator online';
      statusText.textContent = 'C++ Live (8080)';
    } else {
      throw new Error();
    }
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
    const res = await fetch(`${API_URL}?mode=grammar&q=${encodeURIComponent(text)}`);
    if (!res.ok) throw new Error('API request failed');
    const data = await res.json();

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

// Run status check on popup open
checkServerStatus();
