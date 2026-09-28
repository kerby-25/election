const STORAGE_KEY = 'election_ueh_web_v3';
const THEME_KEY = 'election_ueh_theme';

const departments = ['Artibonite','Centre','Grand Anse','Nippes','Nord','Nord-Est','Nord-Ouest','Ouest','Sud','Sud-Est'];
const viewNames = {
  dashboard: ['Tableau de bord','Vue d’ensemble'],
  bvs: ['Bureaux de vote','Bureaux de vote'],
  electeurs: ['Électeurs','Registre des électeurs'],
  candidats: ['Candidats','Candidats à la présidentielle'],
  votes: ['Votes','Registre des votes'],
  partis: ['Partis politiques','Partis politiques'],
  rapports: ['Rapports & résultats','Analyse du scrutin']
};

const app = document.getElementById('app');
const modalBackdrop = document.getElementById('modalBackdrop');
const modalTitle = document.getElementById('modalTitle');
const modalEyebrow = document.getElementById('modalEyebrow');
const modalBody = document.getElementById('modalBody');
const toastRegion = document.getElementById('toastRegion');

const memoryStorage = (() => {
  const data = Object.create(null);
  return {
    getItem: key => Object.prototype.hasOwnProperty.call(data, key) ? data[key] : null,
    setItem: (key, value) => { data[key] = String(value); },
    removeItem: key => { delete data[key]; }
  };
})();

function getStorage() {
  try {
    const s = window.localStorage;
    const probe = '__election_ueh_probe__';
    s.setItem(probe, '1');
    s.removeItem(probe);
    return s;
  } catch (err) {
    return memoryStorage;
  }
}

const storage = getStorage();

let state = loadState();
let currentView = 'dashboard';
let reportTab = 'bvs-dept';
let globalQuery = '';

function emptyState() {
  return { partis: [], bvs: [], electeurs: [], candidats: [], votes: [] };
}

function loadState() {
  try {
    const raw = storage.getItem(STORAGE_KEY);
    if (!raw) return emptyState();
    const parsed = JSON.parse(raw);
    return Object.assign(emptyState(), parsed);
  } catch (err) {
    return emptyState();
  }
}

function saveState() {
  storage.setItem(STORAGE_KEY, JSON.stringify(state));
}

function escapeHtml(value = '') {
  return String(value).replace(/[&<>'"]/g, ch => ({'&':'&amp;','<':'&lt;','>':'&gt;',"'":'&#39;','"':'&quot;'}[ch]));
}

function uid(prefix, collection, validator = null) {
  const used = new Set(collection.map(x => x.id || x.idBV || x.idCandidat || x.idVote || x.idPP || x.ninu));
  for (let n = 1; n < 1000000; n++) {
    const candidate = `${prefix}${String(n).padStart(6, '0')}`;
    if (!used.has(candidate) && (!validator || validator(candidate))) return candidate;
  }
  throw new Error('Impossible de generer un identifiant unique.');
}

function formatDate(value) {
  if (!value) return '—';
  const d = new Date(`${value}T00:00:00`);
  if (Number.isNaN(d.getTime())) return value;
  return d.toLocaleDateString('fr-FR');
}

function todayInput() {
  const d = new Date();
  const m = String(d.getMonth() + 1).padStart(2,'0');
  const day = String(d.getDate()).padStart(2,'0');
  return `${d.getFullYear()}-${m}-${day}`;
}

function ageFromDate(date) {
  const birth = new Date(`${date}T00:00:00`);
  const now = new Date();
  let age = now.getFullYear() - birth.getFullYear();
  const beforeBirthday = now.getMonth() < birth.getMonth() || (now.getMonth() === birth.getMonth() && now.getDate() < birth.getDate());
  if (beforeBirthday) age--;
  return age;
}

function validNinu(value) { return /^\d{10}$/.test(value); }
function validId(value) { return /^[A-Za-z]{2}\d{6}$/.test(value); }
function validPhone(value) { return /^\d{8}$/.test(value); }
function validGmail(value) { return /^[A-Za-z0-9.]+@gmail\.com$/i.test(value) && !value.startsWith('.') && !value.endsWith('.') && !value.includes('..'); }
function notify(message, type='success', title=type === 'error' ? 'Attention' : 'Opération réussie') {
  const node = document.createElement('div');
  node.className = `toast ${type === 'error' ? 'error' : ''}`;
  node.innerHTML = `<strong>${escapeHtml(title)}</strong><div>${escapeHtml(message)}</div>`;
  toastRegion.appendChild(node);
  setTimeout(() => node.remove(), 3400);
}

function setView(view) {
  currentView = view;
  document.querySelectorAll('.nav-item[data-view]').forEach(btn => btn.classList.toggle('is-active', btn.dataset.view === view));
  document.getElementById('viewLabel').textContent = viewNames[view][0];
  document.getElementById('pageTitle').textContent = viewNames[view][1];
  document.getElementById('sidebar').classList.remove('open');
  render();
}

document.querySelectorAll('.nav-item[data-view]').forEach(btn => btn.addEventListener('click', () => setView(btn.dataset.view)));
document.getElementById('menuToggle').addEventListener('click', () => document.getElementById('sidebar').classList.toggle('open'));
document.getElementById('themeToggle').addEventListener('click', () => {
  const root = document.documentElement;
  const next = root.dataset.theme === 'dark' ? 'light' : 'dark';
  root.dataset.theme = next;
  storage.setItem(THEME_KEY, next);
  document.getElementById('themeToggle').innerHTML = `<span class="icon ${next === 'dark' ? 'icon-sun' : 'icon-moon'}"></span>`;
});

const savedTheme = storage.getItem(THEME_KEY) || 'light';
document.documentElement.dataset.theme = savedTheme;
document.getElementById('themeToggle').innerHTML = `<span class="icon ${savedTheme === 'dark' ? 'icon-sun' : 'icon-moon'}"></span>`;

document.getElementById('demoBtn').addEventListener('click', () => {
  if (state.bvs.length || state.electeurs.length || state.candidats.length || state.votes.length || state.partis.length) {
    if (!confirm('Charger la démonstration remplacera les données locales actuelles. Continuer ?')) return;
  }
  state = demoData();
  saveState();
  notify('Les données de démonstration fictives sont prêtes.');
  render();
});

document.querySelector('[data-close-modal]').addEventListener('click', closeModal);
modalBackdrop.addEventListener('click', e => { if (e.target === modalBackdrop) closeModal(); });

document.getElementById('globalSearchBtn').addEventListener('click', openGlobalSearch);
document.addEventListener('keydown', e => {
  if (e.key === 'Escape') closeModal();
  if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); openGlobalSearch(); }
});

function openModal({title, eyebrow='Action', body, onSubmit, submitLabel='Enregistrer'}) {
  modalTitle.textContent = title;
  modalEyebrow.textContent = eyebrow;
  modalBody.innerHTML = body;
  modalBackdrop.hidden = false;
  const form = modalBody.querySelector('form');
  if (form && onSubmit) form.addEventListener('submit', e => { e.preventDefault(); onSubmit(new FormData(form), form); });
  setTimeout(() => modalBody.querySelector('input, select, textarea')?.focus(), 20);
  modalBody.querySelectorAll('[data-cancel-modal]').forEach(btn => btn.addEventListener('click', closeModal));
}

function closeModal() { modalBackdrop.hidden = true; modalBody.innerHTML = ''; }

function addressFields(prefix, departmentValue='') {
  return `
    <div class="field"><label for="${prefix}-noRue">No rue</label><input id="${prefix}-noRue" name="${prefix}NoRue" inputmode="numeric" maxlength="3" required placeholder="12"></div>
    <div class="field"><label for="${prefix}-rue">Rue</label><input id="${prefix}-rue" name="${prefix}Rue" maxlength="100" required placeholder="Rue des Écoles"></div>
    <div class="field full"><label for="${prefix}-dept">Département</label><input id="${prefix}-dept" name="${prefix}Dept" list="deptList" value="${escapeHtml(departmentValue)}" required placeholder="Ouest"></div>`;
}

function render() {
  if (currentView === 'dashboard') renderDashboard();
  if (currentView === 'bvs') renderBVs();
  if (currentView === 'electeurs') renderElecteurs();
  if (currentView === 'candidats') renderCandidats();
  if (currentView === 'votes') renderVotes();
  if (currentView === 'partis') renderPartis();
  if (currentView === 'rapports') renderRapports();
}

function countByDept(items, getDept) {
  const map = new Map();
  items.forEach(x => { const d = getDept(x) || 'Non renseigné'; map.set(d, (map.get(d)||0)+1); });
  return [...map.entries()].sort((a,b)=>b[1]-a[1]);
}

function renderDashboard() {
  const deptRows = countByDept(state.electeurs, e => e.adresse?.departement).slice(0,7);
  const max = deptRows.length ? deptRows[0][1] : 1;
  const voted = new Set(state.votes.map(v => v.ninu)).size;
  const turnoutBase = state.electeurs.length || 1;
  const participation = Math.min(100, voted / turnoutBase * 100);
  const lastVotes = [...state.votes].sort((a,b)=>`${b.dateVote}`.localeCompare(`${a.dateVote}`)).slice(0,5);
  app.innerHTML = `
    <datalist id="deptList">${departments.map(d=>`<option value="${escapeHtml(d)}">`).join('')}</datalist>
    <div class="hero">
      <div class="hero-main">
        <span class="eyebrow">Système de gestion électorale</span>
        <h1>Une vue claire sur tout le processus électoral.</h1>
        <p>Suivez les bureaux de vote, les électeurs, les candidatures et les votes dans une interface moderne pensée pour la démonstration du projet UEH.</p>
        <div class="hero-meta">
          <span class="pill"><i class="pill-dot"></i>Données locales actives</span>
          <span class="pill">Aujourd’hui · ${formatDate(todayInput())}</span>
          <span class="pill">5 modules · 2 rapports géographiques</span>
        </div>
      </div>
      <div class="hero-side">
        <div class="mini-card">
          <span class="eyebrow">Participation enregistrée</span>
          <div class="big-number">${participation.toFixed(1)}%</div>
          <p>${voted} électeur(s) avec un vote enregistré sur ${state.electeurs.length}.</p>
          <div class="mini-foot"><span>Base actuelle</span><strong>${state.electeurs.length}</strong></div>
        </div>
        <div class="mini-card">
          <span class="eyebrow">Prochaine action logique</span>
          <div style="font-size:17px;font-weight:850;margin-top:8px;">${state.bvs.length ? 'Ajouter les électeurs' : 'Créer le premier BV'}</div>
          <p>${state.bvs.length ? 'Chaque BV peut inscrire ses trois responsables directement à sa création.' : 'Le premier BV débloque l’inscription des électeurs.'}</p>
          <div class="mini-foot"><span>Flux recommandé</span><span class="badge success">Cohérent</span></div>
        </div>
      </div>
    </div>

    <div class="stats-grid">
      ${statCard('building','Bureaux de vote',state.bvs.length,'Structure territoriale')}
      ${statCard('users','Électeurs',state.electeurs.length,'Registre dynamique')}
      ${statCard('candidate','Candidats',state.candidats.length,'Candidatures')}
      ${statCard('vote','Votes',state.votes.length,'Historique enregistré')}
      ${statCard('flag','Partis',state.partis.length,'Partis politiques')}
    </div>

    <div class="grid-2">
      <section class="panel">
        <div class="panel-head"><div><h3>Électeurs par département</h3><p>Lecture rapide de la répartition géographique.</p></div><button class="btn btn-soft" onclick="setView('rapports')">Ouvrir les rapports</button></div>
        <div class="panel-body">
          ${deptRows.length ? `<div class="chart-bars">${deptRows.map(([d,n])=>`<div class="bar-row"><span class="label">${escapeHtml(d)}</span><div class="bar"><span style="width:${Math.max(6,n/max*100)}%"></span></div><span class="bar-value">${n}</span></div>`).join('')}</div>` : `<div class="empty-state"><strong>Aucune donnée géographique</strong>Ajoutez un BV puis des électeurs pour voir la répartition.</div>`}
        </div>
      </section>
      <section class="panel">
        <div class="panel-head"><div><h3>Derniers votes</h3><p>Les enregistrements les plus récents.</p></div><button class="btn btn-soft" onclick="setView('votes')">Tout voir</button></div>
        <div class="panel-body">
          ${lastVotes.length ? lastVotes.map(v=>{const c=v.idCandidat=== 'BLANC' ? 'Vote blanc' : (state.candidats.find(c=>c.idCandidat===v.idCandidat)?.nom || v.idCandidat); return `<div style="display:flex;justify-content:space-between;gap:12px;padding:11px 0;border-bottom:1px solid var(--line);"><div><strong style="font-size:12px;">${escapeHtml(v.ninu)}</strong><div style="color:var(--muted);font-size:10px;">${escapeHtml(v.idBV)} · ${formatDate(v.dateVote)}</div></div><span class="badge ${v.idCandidat==='BLANC'?'gold':'success'}">${escapeHtml(c)}</span></div>`}).join('') : `<div class="empty-state"><strong>Aucun vote</strong>Le registre des votes apparaîtra ici.</div>`}
        </div>
      </section>
    </div>`;
}

function statCard(icon, label, value, note) {
  const iconClass = icon === 'users' ? 'icon-users' : icon === 'building' ? 'icon-building' : icon === 'candidate' ? 'icon-candidate' : icon === 'vote' ? 'icon-vote' : 'icon-flag';
  return `<div class="stat-card"><div class="stat-top"><div class="stat-icon"><span class="icon ${iconClass}"></span></div><span class="stat-trend">ACTIF</span></div><strong>${value}</strong><span>${escapeHtml(label)} · ${escapeHtml(note)}</span></div>`;
}

function renderBVs() {
  app.innerHTML = `
    <datalist id="deptList">${departments.map(d=>`<option value="${escapeHtml(d)}">`).join('')}</datalist>
    <div class="view-toolbar">
      <div><h1>Bureaux de vote</h1><p>Création intégrée : le BV et ses trois responsables deviennent des électeurs dans la même opération.</p></div>
      <div class="toolbar-actions"><div class="search-box"><span class="icon icon-search"></span><input id="bvSearch" placeholder="Rechercher un BV, une rue, un département…"></div><select class="select" id="bvDeptFilter"><option value="">Tous les départements</option>${departments.map(d=>`<option>${d}</option>`).join('')}</select><button class="btn btn-primary" onclick="openBVModal()"><span class="icon icon-plus"></span>Nouveau BV</button></div>
    </div>
    <section class="panel table-panel"><table class="data-table"><thead><tr><th>ID</th><th>Département</th><th>Adresse</th><th>Responsables</th><th>Électeurs</th><th></th></tr></thead><tbody id="bvRows"></tbody></table></section>`;
  document.getElementById('bvSearch').addEventListener('input', fillBVRows);
  document.getElementById('bvDeptFilter').addEventListener('change', fillBVRows);
  fillBVRows();
}

function fillBVRows() {
  const q = (document.getElementById('bvSearch')?.value || '').toLowerCase();
  const dept = document.getElementById('bvDeptFilter')?.value || '';
  const rows = state.bvs.filter(b => `${b.idBV} ${b.adresse?.rue} ${b.adresse?.departement}`.toLowerCase().includes(q) && (!dept || b.adresse?.departement === dept));
  const target = document.getElementById('bvRows');
  if (!target) return;
  target.innerHTML = rows.length ? rows.map(b => {
    const n = state.electeurs.filter(e=>e.idBV===b.idBV).length;
    return `<tr><td class="code">${escapeHtml(b.idBV)}</td><td><span class="badge">${escapeHtml(b.adresse?.departement || '—')}</span></td><td>${b.adresse?.noRue || '—'}, ${escapeHtml(b.adresse?.rue || '')}</td><td class="name-cell"><strong>${escapeHtml(responsibleName(b.responsables?.president))}</strong><span>Vice : ${escapeHtml(responsibleName(b.responsables?.vice))} · Secrétaire : ${escapeHtml(responsibleName(b.responsables?.secretaire))}</span></td><td><span class="badge success">${n} électeur(s)</span></td><td><button class="btn" onclick="viewBV('${escapeHtml(b.idBV)}')">Détails</button> <button class="btn btn-danger" onclick="deleteBV('${escapeHtml(b.idBV)}')">Suppr.</button></td></tr>`;
  }).join('') : `<tr><td colspan="6"><div class="empty-state"><strong>Aucun bureau trouvé</strong>Créez un BV pour démarrer le registre.</div></td></tr>`;
}

function responsibleName(ninu) {
  const e = state.electeurs.find(e=>e.ninu===ninu);
  return e ? `${e.nom} ${e.prenom}` : (ninu || 'Non affecté');
}

function openBVModal() {
  const section = role => `
    <div class="responsable-box">
      <h4>${role}</h4>
      <div class="form-grid">
        <div class="field"><label>NINU</label><input name="${role}Ninu" inputmode="numeric" maxlength="10" required placeholder="10 chiffres"></div>
        <div class="field"><label>Nom</label><input name="${role}Nom" maxlength="79" required></div>
        <div class="field"><label>Prénom</label><input name="${role}Prenom" maxlength="79" required></div>
        <div class="field"><label>Date de naissance</label><input type="date" name="${role}Naissance" required></div>
      </div>
      <div class="form-grid" style="margin-top:12px">${addressFields(role)}</div>
    </div>`;
  openModal({title:'Créer un bureau de vote', eyebrow:'BV + responsables', body:`
    <form>
      <div class="form-grid">
        <div class="field full"><label>ID du BV · généré automatiquement</label><input value="Génération automatique après validation" disabled></div>
        <div class="field"><label>No rue du BV</label><input name="bvNoRue" inputmode="numeric" maxlength="3" required></div>
        <div class="field"><label>Rue du BV</label><input name="bvRue" maxlength="100" required placeholder="Rue principale"></div>
        <div class="field full"><label>Département du BV</label><input name="bvDept" list="deptList" required placeholder="Ouest"></div>
        <div class="section-title">Responsables inscrits comme électeurs</div>
        <div class="section-note">Chaque responsable est créé immédiatement dans le registre des électeurs et reçoit le même ID_BV. Les trois NINU doivent être différents.</div>
        <div class="field full">${section('Président')}</div>
        <div class="field full">${section('Vice')}</div>
        <div class="field full">${section('Secretaire')}</div>
      </div>
      <div class="modal-actions"><button type="button" class="btn" data-cancel-modal>Annuler</button><button type="submit" class="btn btn-primary">Créer le BV</button></div>
    </form>`, onSubmit: createBV });
}

function collectAddress(fd, prefix) {
  return {noRue:Number(fd.get(`${prefix}NoRue`)), rue:String(fd.get(`${prefix}Rue`)).trim(), departement:String(fd.get(`${prefix}Dept`)).trim()};
}

function createBV(fd) {
  const bvNo = String(fd.get('bvNoRue')).trim();
  const bvRue = String(fd.get('bvRue')).trim();
  const bvDept = String(fd.get('bvDept')).trim();
  if (!/^\d{1,3}$/.test(bvNo) || !bvRue || !bvDept) return notify('Les informations du BV sont obligatoires et le numéro de rue doit contenir 1 à 3 chiffres.','error');
  const roles = [['Président','president'],['Vice','vice'],['Secretaire','secretaire']];
  const ninus = [];
  const newElecteurs = [];
  for (const [label,key] of roles) {
    const ninu = String(fd.get(`${label}Ninu`)).trim();
    if (!validNinu(ninu)) return notify(`Le NINU du ${label.toLowerCase()} doit contenir exactement 10 chiffres.`,'error');
    if (ninu !== String(fd.get('ViceNinu')||'').trim() && key === 'president') {}
    if (ninus.includes(ninu)) return notify('Les trois responsables doivent avoir des NINU distincts.','error');
    if (state.electeurs.some(e=>e.ninu===ninu)) return notify(`Le NINU ${ninu} est déjà inscrit comme électeur.`,'error');
    const naissance = String(fd.get(`${label}Naissance`));
    if (!naissance || ageFromDate(naissance) < 18) return notify(`Le ${label.toLowerCase()} doit avoir au moins 18 ans.`,'error');
    const address = collectAddress(fd,label);
    if (!Number.isInteger(address.noRue) || address.noRue < 1 || address.noRue > 999 || !/^[A-Za-zÀ-ÿ .'-]+$/.test(address.rue) || !address.departement) return notify(`Adresse invalide pour le ${label.toLowerCase()}.`,'error');
    newElecteurs.push({ninu, nom:String(fd.get(`${label}Nom`)).trim(), prenom:String(fd.get(`${label}Prenom`)).trim(), adresse:address, idBV:'', dateNaissance:naissance});
    ninus.push(ninu);
  }
  const idBV = uid('BV', state.bvs);
  newElecteurs.forEach(e=>e.idBV=idBV);
  const bv = {idBV, adresse:{noRue:Number(bvNo), rue:bvRue, departement:bvDept}, responsables:{president:ninus[0],vice:ninus[1],secretaire:ninus[2]}, membresAffectes:true};
  state.bvs.push(bv);
  state.electeurs.push(...newElecteurs);
  saveState(); closeModal(); renderBVs(); notify(`BV ${idBV} créé avec ses trois responsables inscrits comme électeurs.`); 
}

function viewBV(id) {
  const b = state.bvs.find(x=>x.idBV===id); if(!b) return;
  const elects = state.electeurs.filter(e=>e.idBV===id);
  openModal({title:`Bureau ${id}`, eyebrow:'Détail du bureau', body:`<div style="padding:20px 24px 24px"><div class="summary-grid"><div class="summary-card"><span>Département</span><strong>${escapeHtml(b.adresse.departement)}</strong></div><div class="summary-card"><span>Adresse</span><strong>${b.adresse.noRue}</strong></div><div class="summary-card"><span>Électeurs</span><strong>${elects.length}</strong></div><div class="summary-card"><span>État</span><strong>Actif</strong></div></div><div class="group-title">Responsables</div><div class="table-panel"><table class="data-table"><thead><tr><th>Fonction</th><th>NINU</th><th>Nom</th></tr></thead><tbody>${[['Président',b.responsables.president],['Vice-président',b.responsables.vice],['Secrétaire',b.responsables.secretaire]].map(([r,n])=>{const e=state.electeurs.find(x=>x.ninu===n);return `<tr><td>${r}</td><td class="code">${escapeHtml(n)}</td><td>${e?escapeHtml(e.nom+' '+e.prenom):'—'}</td></tr>`}).join('')}</tbody></table></div><div class="modal-actions"><button class="btn" data-close-modal-detail>Fermer</button></div></div>`});
  modalBody.querySelector('[data-close-modal-detail]')?.addEventListener('click', closeModal);
}

function deleteBV(id) {
  const b = state.bvs.find(x=>x.idBV===id); if(!b) return;
  if (state.votes.some(v=>v.idBV===id)) return notify('Impossible de supprimer ce BV : des votes le référencent.','error');
  if (state.electeurs.some(e=>e.idBV===id)) return notify('Impossible de supprimer ce BV : ses électeurs doivent d’abord être supprimés ou déplacés.','error');
  if (!confirm(`Supprimer le BV ${id} ?`)) return;
  state.bvs = state.bvs.filter(x=>x.idBV!==id); saveState(); renderBVs(); notify(`BV ${id} supprimé.`);
}

function renderElecteurs() {
  app.innerHTML = `
    <div class="view-toolbar">
      <div><h1>Électeurs</h1><p>Registre dynamique. Les responsables créés avec un BV apparaissent ici immédiatement.</p></div>
      <div class="toolbar-actions"><div class="search-box"><span class="icon icon-search"></span><input id="electSearch" placeholder="NINU, nom, prénom, département…"></div><select class="select" id="electDept"><option value="">Tous les départements</option>${departments.map(d=>`<option>${d}</option>`).join('')}</select><select class="select" id="electBV"><option value="">Tous les BV</option>${state.bvs.map(b=>`<option value="${b.idBV}">${b.idBV}</option>`).join('')}</select><button class="btn btn-primary" onclick="openElecteurModal()"><span class="icon icon-plus"></span>Nouvel électeur</button></div>
    </div>
    <section class="panel table-panel"><table class="data-table"><thead><tr><th>NINU</th><th>Électeur</th><th>Naissance</th><th>Département</th><th>BV</th><th></th></tr></thead><tbody id="electRows"></tbody></table></section>`;
  ['electSearch','electDept','electBV'].forEach(id=>document.getElementById(id).addEventListener('input',fillElectRows));
  fillElectRows();
}
function fillElectRows() {
  const q=(document.getElementById('electSearch')?.value||'').toLowerCase(), d=document.getElementById('electDept')?.value||'', bv=document.getElementById('electBV')?.value||'';
  const rows=state.electeurs.filter(e=>`${e.ninu} ${e.nom} ${e.prenom} ${e.adresse?.departement} ${e.idBV}`.toLowerCase().includes(q)&&(!d||e.adresse?.departement===d)&&(!bv||e.idBV===bv));
  document.getElementById('electRows').innerHTML=rows.length?rows.map(e=>`<tr><td class="code">${escapeHtml(e.ninu)}</td><td class="name-cell"><strong>${escapeHtml(e.nom)} ${escapeHtml(e.prenom)}</strong><span>Âge ${ageFromDate(e.dateNaissance)} ans</span></td><td>${formatDate(e.dateNaissance)}</td><td><span class="badge">${escapeHtml(e.adresse?.departement||'—')}</span></td><td class="code">${escapeHtml(e.idBV||'—')}</td><td><button class="btn" onclick="viewElecteur('${escapeHtml(e.ninu)}')">Détails</button> <button class="btn btn-danger" onclick="deleteElecteur('${escapeHtml(e.ninu)}')">Suppr.</button></td></tr>`).join(''):`<tr><td colspan="6"><div class="empty-state"><strong>Aucun électeur</strong>Utilisez « Nouvel électeur » ou créez un BV avec ses responsables.</div></td></tr>`;
}
function openElecteurModal() {
  openModal({title:'Inscrire un électeur',eyebrow:'Registre électoral',body:`<form><div class="form-grid"><div class="field"><label>NINU</label><input name="ninu" inputmode="numeric" maxlength="10" required></div><div class="field"><label>Nom</label><input name="nom" required></div><div class="field"><label>Prénom</label><input name="prenom" required></div><div class="field"><label>Date de naissance</label><input type="date" name="naissance" required></div><div class="section-title">Adresse</div>${addressFields('elect') }<div class="field full"><label>Bureau de vote</label><select name="idBV" required><option value="">Sélectionner un BV</option>${state.bvs.map(b=>`<option>${b.idBV}</option>`).join('')}</select></div></div><div class="modal-actions"><button type="button" class="btn" data-cancel-modal>Annuler</button><button class="btn btn-primary">Enregistrer l’électeur</button></div></form>`,onSubmit:createElecteur});
}
function createElecteur(fd){
  const n=String(fd.get('ninu')).trim(); if(!validNinu(n)) return notify('Le NINU doit contenir exactement 10 chiffres.','error'); if(state.electeurs.some(e=>e.ninu===n)) return notify('Ce NINU existe déjà.','error');
  const naissance=String(fd.get('naissance')); if(ageFromDate(naissance)<18) return notify('L’électeur doit avoir au moins 18 ans.','error');
  const address=collectAddress(fd,'elect'); const idBV=String(fd.get('idBV')); if(!state.bvs.some(b=>b.idBV===idBV)) return notify('Le BV sélectionné n’existe pas.','error');
  state.electeurs.push({ninu:n,nom:String(fd.get('nom')).trim(),prenom:String(fd.get('prenom')).trim(),dateNaissance:naissance,adresse:address,idBV}); saveState(); closeModal(); renderElecteurs(); notify(`Électeur ${n} enregistré.`);
}
function viewElecteur(n){const e=state.electeurs.find(x=>x.ninu===n);if(!e)return;openModal({title:`Électeur ${n}`,eyebrow:'Fiche électeur',body:`<div style="padding:20px 24px 24px"><div class="summary-grid"><div class="summary-card"><span>Nom</span><strong>${escapeHtml(e.nom)}</strong></div><div class="summary-card"><span>Prénom</span><strong>${escapeHtml(e.prenom)}</strong></div><div class="summary-card"><span>Âge</span><strong>${ageFromDate(e.dateNaissance)}</strong></div><div class="summary-card"><span>BV</span><strong>${escapeHtml(e.idBV)}</strong></div></div><div class="group-title">Adresse</div><div class="panel"><div class="panel-body"><strong>${e.adresse.noRue}, ${escapeHtml(e.adresse.rue)}</strong><div style="color:var(--muted);margin-top:3px;">${escapeHtml(e.adresse.departement)}</div></div></div><div class="modal-actions"><button class="btn" data-close-modal-detail>Fermer</button></div></div>`});modalBody.querySelector('[data-close-modal-detail]')?.addEventListener('click',closeModal);}
function deleteElecteur(n){if(state.votes.some(v=>v.ninu===n)||state.bvs.some(b=>Object.values(b.responsables||{}).includes(n)))return notify('Impossible de supprimer cet électeur : il est référencé par un vote ou comme responsable de BV.','error');if(!confirm(`Supprimer l’électeur ${n} ?`))return;state.electeurs=state.electeurs.filter(e=>e.ninu!==n);saveState();renderElecteurs();notify(`Électeur ${n} supprimé.`);}

function renderCandidats(){
  app.innerHTML=`<div class="view-toolbar"><div><h1>Candidats</h1><p>Suivi des candidatures indépendantes ou rattachées à un parti.</p></div><div class="toolbar-actions"><div class="search-box"><span class="icon icon-search"></span><input id="candSearch" placeholder="ID, nom, NINU…"></div><button class="btn btn-primary" onclick="openCandidatModal()"><span class="icon icon-plus"></span>Nouveau candidat</button></div></div><section class="panel table-panel"><table class="data-table"><thead><tr><th>ID</th><th>Candidat</th><th>NINU</th><th>Type</th><th>Parti</th><th></th></tr></thead><tbody id="candRows"></tbody></table></section>`;
  document.getElementById('candSearch').addEventListener('input',fillCandRows);fillCandRows();
}
function fillCandRows(){const q=(document.getElementById('candSearch')?.value||'').toLowerCase();const rows=state.candidats.filter(c=>`${c.idCandidat} ${c.nom} ${c.prenom} ${c.ninu} ${c.type}`.toLowerCase().includes(q));document.getElementById('candRows').innerHTML=rows.length?rows.map(c=>`<tr><td class="code">${escapeHtml(c.idCandidat)}</td><td class="name-cell"><strong>${escapeHtml(c.nom)} ${escapeHtml(c.prenom)}</strong><span>${formatDate(c.dateNaissance)} · ${ageFromDate(c.dateNaissance)} ans</span></td><td class="code">${escapeHtml(c.ninu)}</td><td><span class="badge ${c.type==='0'?'gold':'success'}">${c.type==='0'?'Indépendant':escapeHtml(c.type)}</span></td><td>${c.type==='0'?'—':escapeHtml(c.type)}</td><td><button class="btn btn-danger" onclick="deleteCandidat('${escapeHtml(c.idCandidat)}')">Suppr.</button></td></tr>`).join(''):`<tr><td colspan="6"><div class="empty-state"><strong>Aucun candidat</strong>Ajoutez une candidature pour alimenter les résultats.</div></td></tr>`;}
function openCandidatModal(){openModal({title:'Inscrire un candidat',eyebrow:'Candidature',body:`<form><div class="form-grid"><div class="field"><label>ID · automatique</label><input value="Génération automatique" disabled></div><div class="field"><label>NINU</label><input name="ninu" maxlength="10" inputmode="numeric" required></div><div class="field"><label>Nom</label><input name="nom" required></div><div class="field"><label>Prénom</label><input name="prenom" required></div><div class="field"><label>Date de naissance</label><input type="date" name="naissance" required></div><div class="field"><label>Type</label><select name="type"><option value="0">Indépendant</option>${state.partis.map(p=>`<option value="${p.idPP}">${escapeHtml(p.idPP)} · ${escapeHtml(p.nom)}</option>`).join('')}</select></div><div class="section-title">Coordonnées</div>${addressFields('cand')}<div class="field"><label>Téléphone</label><input name="telephone" inputmode="numeric" maxlength="8" required></div><div class="field"><label>Email Gmail</label><input type="email" name="email" required placeholder="nom@gmail.com"></div></div><div class="modal-actions"><button type="button" class="btn" data-cancel-modal>Annuler</button><button class="btn btn-primary">Enregistrer</button></div></form>`,onSubmit:createCandidat});}
function createCandidat(fd){const n=String(fd.get('ninu')).trim();if(!validNinu(n)||state.candidats.some(c=>c.ninu===n))return notify('NINU invalide ou déjà utilisé par un candidat.','error');const naissance=String(fd.get('naissance'));if(ageFromDate(naissance)<25)return notify('Le candidat doit avoir au moins 25 ans.','error');if(!validPhone(String(fd.get('telephone')).trim()))return notify('Le téléphone doit contenir 8 chiffres.','error');if(!validGmail(String(fd.get('email')).trim()))return notify('Utilisez une adresse Gmail valide.','error');const type=String(fd.get('type'));if(type!=='0'&&!state.partis.some(p=>p.idPP===type))return notify('Parti introuvable.','error');state.candidats.push({idCandidat:uid('CA',state.candidats),ninu:n,nom:String(fd.get('nom')).trim(),prenom:String(fd.get('prenom')).trim(),dateNaissance:naissance,adresse:collectAddress(fd,'cand'),email:String(fd.get('email')).trim(),telephone:String(fd.get('telephone')).trim(),type,dateInscription:todayInput()});saveState();closeModal();renderCandidats();notify(`Candidat ${n} enregistré.`);}
function deleteCandidat(id){if(state.votes.some(v=>v.idCandidat===id))return notify('Impossible de supprimer ce candidat : des votes le référencent.','error');if(!confirm(`Supprimer ${id} ?`))return;state.candidats=state.candidats.filter(c=>c.idCandidat!==id);saveState();renderCandidats();notify(`${id} supprimé.`);}

function renderVotes(){
  app.innerHTML=`<div class="view-toolbar"><div><h1>Votes</h1><p>Un électeur ne peut voter qu’une seule fois. La date est enregistrée automatiquement.</p></div><div class="toolbar-actions"><div class="search-box"><span class="icon icon-search"></span><input id="voteSearch" placeholder="NINU, BV, candidat…"></div><button class="btn btn-primary" onclick="openVoteModal()"><span class="icon icon-plus"></span>Enregistrer un vote</button></div></div><section class="panel table-panel"><table class="data-table"><thead><tr><th>ID vote</th><th>Date</th><th>BV</th><th>Électeur</th><th>Choix</th></tr></thead><tbody id="voteRows"></tbody></table></section>`;
  document.getElementById('voteSearch').addEventListener('input',fillVoteRows);fillVoteRows();
}
function fillVoteRows(){const q=(document.getElementById('voteSearch')?.value||'').toLowerCase();const rows=state.votes.filter(v=>`${v.idVote} ${v.ninu} ${v.idBV} ${v.idCandidat}`.toLowerCase().includes(q));document.getElementById('voteRows').innerHTML=rows.length?rows.map(v=>{const label=v.idCandidat==='BLANC'?'Vote blanc':(state.candidats.find(c=>c.idCandidat===v.idCandidat)?.nom||v.idCandidat);return `<tr><td class="code">${escapeHtml(v.idVote)}</td><td>${formatDate(v.dateVote)}</td><td class="code">${escapeHtml(v.idBV)}</td><td class="code">${escapeHtml(v.ninu)}</td><td><span class="badge ${v.idCandidat==='BLANC'?'gold':'success'}">${escapeHtml(label)}</span></td></tr>`}).join(''):`<tr><td colspan="5"><div class="empty-state"><strong>Aucun vote</strong>Enregistrez un vote ou chargez la démo.</div></td></tr>`;}
function openVoteModal(){const available=state.electeurs.filter(e=>!state.votes.some(v=>v.ninu===e.ninu));openModal({title:'Enregistrer un vote',eyebrow:'Vote',body:`<form><div class="form-grid"><div class="field full"><label>Électeur</label><select name="ninu" required><option value="">Sélectionner un électeur</option>${available.map(e=>`<option value="${e.ninu}">${e.ninu} · ${escapeHtml(e.nom)} ${escapeHtml(e.prenom)} · ${e.idBV}</option>`).join('')}</select></div><div class="field full"><label>Choix du candidat</label><select name="candidate" required><option value="">Sélectionner un candidat</option>${state.candidats.map(c=>`<option value="${c.idCandidat}">${c.idCandidat} · ${escapeHtml(c.nom)} ${escapeHtml(c.prenom)} · ${c.ninu}</option>`).join('')}<option value="BLANC">Vote blanc</option></select></div><div class="field"><label>Date du vote</label><input value="${formatDate(todayInput())}" disabled></div><div class="field"><label>ID_VOTE</label><input value="Généré automatiquement" disabled></div></div><div class="modal-actions"><button type="button" class="btn" data-cancel-modal>Annuler</button><button class="btn btn-primary">Enregistrer le vote</button></div></form>`,onSubmit:createVote});}
function createVote(fd){const n=String(fd.get('ninu'));if(!n||state.votes.some(v=>v.ninu===n))return notify('Cet électeur a déjà voté.','error');const e=state.electeurs.find(x=>x.ninu===n);if(!e)return notify('Électeur introuvable.','error');const c=String(fd.get('candidate'));if(c!=='BLANC'&&!state.candidats.some(x=>x.idCandidat===c))return notify('Candidat introuvable.','error');state.votes.push({idVote:uid('VT',state.votes),dateVote:todayInput(),idBV:e.idBV,ninu:n,idCandidat:c});saveState();closeModal();renderVotes();notify(`Vote ${n} enregistré.`);}

function renderPartis(){app.innerHTML=`<div class="view-toolbar"><div><h1>Partis politiques</h1><p>Partis, responsables et coordonnées.</p></div><div class="toolbar-actions"><div class="search-box"><span class="icon icon-search"></span><input id="partySearch" placeholder="ID ou nom…"></div><button class="btn btn-primary" onclick="openPartyModal()"><span class="icon icon-plus"></span>Nouveau parti</button></div></div><section class="panel table-panel"><table class="data-table"><thead><tr><th>ID</th><th>Parti</th><th>Création</th><th>Responsable</th><th>Téléphone</th><th>Email</th><th></th></tr></thead><tbody id="partyRows"></tbody></table></section>`;document.getElementById('partySearch').addEventListener('input',fillPartyRows);fillPartyRows();}
function fillPartyRows(){const q=(document.getElementById('partySearch')?.value||'').toLowerCase();const rows=state.partis.filter(p=>`${p.idPP} ${p.nom} ${p.responsable}`.toLowerCase().includes(q));document.getElementById('partyRows').innerHTML=rows.length?rows.map(p=>`<tr><td class="code">${escapeHtml(p.idPP)}</td><td><strong>${escapeHtml(p.nom)}</strong></td><td>${formatDate(p.dateCreation)}</td><td>${escapeHtml(p.responsable)}</td><td>${escapeHtml(p.telephone)}</td><td>${escapeHtml(p.email)}</td><td><button class="btn btn-danger" onclick="deleteParty('${escapeHtml(p.idPP)}')">Suppr.</button></td></tr>`).join(''):`<tr><td colspan="7"><div class="empty-state"><strong>Aucun parti</strong>Créez un parti pour l’utiliser lors de l’inscription d’un candidat.</div></td></tr>`;}
function openPartyModal(){openModal({title:'Inscrire un parti politique',eyebrow:'Parti',body:`<form><div class="form-grid"><div class="field"><label>ID · automatique</label><input value="Génération automatique" disabled></div><div class="field"><label>Nom</label><input name="nom" required></div><div class="field"><label>Date de création</label><input type="date" name="creation" max="${todayInput()}" required></div><div class="field"><label>Date d’inscription</label><input type="date" name="inscription" max="${todayInput()}" value="${todayInput()}" required></div><div class="section-title">Adresse et responsable</div>${addressFields('party')}<div class="field"><label>Responsable</label><input name="responsable" required></div><div class="field"><label>Téléphone</label><input name="telephone" inputmode="numeric" maxlength="8" required></div><div class="field"><label>Email Gmail</label><input name="email" type="email" required></div></div><div class="modal-actions"><button type="button" class="btn" data-cancel-modal>Annuler</button><button class="btn btn-primary">Enregistrer</button></div></form>`,onSubmit:createParty});}
function createParty(fd){if(!validPhone(String(fd.get('telephone')).trim()))return notify('Le téléphone doit contenir 8 chiffres.','error');if(!validGmail(String(fd.get('email')).trim()))return notify('Utilisez une adresse Gmail valide.','error');const cr=String(fd.get('creation')),ins=String(fd.get('inscription'));if(cr>todayInput()||ins>todayInput())return notify('Les dates futures sont interdites.','error');state.partis.push({idPP:uid('PP',state.partis),nom:String(fd.get('nom')).trim(),dateCreation:cr,dateInscription:ins,adresse:collectAddress(fd,'party'),responsable:String(fd.get('responsable')).trim(),telephone:String(fd.get('telephone')).trim(),email:String(fd.get('email')).trim()});saveState();closeModal();renderPartis();notify('Parti enregistré.');}
function deleteParty(id){if(state.candidats.some(c=>c.type===id))return notify('Impossible de supprimer ce parti : un candidat y est rattaché.','error');if(!confirm(`Supprimer ${id} ?`))return;state.partis=state.partis.filter(p=>p.idPP!==id);saveState();renderPartis();notify(`${id} supprimé.`);}

function renderRapports(){
  app.innerHTML=`<div class="view-toolbar"><div><h1>Rapports & résultats</h1><p>Les deux vues géographiques demandées sont regroupées ici, avec les résultats par département et au niveau national.</p></div><div class="toolbar-actions"><button class="btn" onclick="exportJSON()"><span class="icon icon-download"></span>Exporter</button><button class="btn" onclick="document.getElementById('importInput').click()"><span class="icon icon-upload"></span>Importer</button></div></div>
  <div class="report-tabs"><button class="tab-btn ${reportTab==='bvs-dept'?'active':''}" onclick="setReportTab('bvs-dept')">BV par département</button><button class="tab-btn ${reportTab==='elect-dept'?'active':''}" onclick="setReportTab('elect-dept')">Électeurs par département / BV</button><button class="tab-btn ${reportTab==='results'?'active':''}" onclick="setReportTab('results')">Résultats</button></div><div id="reportContent" style="margin-top:18px"></div>`;
  fillReport();
}
function setReportTab(tab){reportTab=tab;renderRapports();}
function fillReport(){const target=document.getElementById('reportContent');if(!target)return;if(reportTab==='bvs-dept')return fillBVByDept(target);if(reportTab==='elect-dept')return fillElectByDept(target);fillResults(target);}
function fillBVByDept(target){const groups=countByDept(state.bvs,b=>b.adresse?.departement);if(!groups.length){target.innerHTML=`<section class="panel"><div class="empty-state"><strong>Aucun bureau de vote</strong>Créez un BV pour obtenir un rapport territorial.</div></section>`;return;}target.innerHTML=groups.map(([dept])=>{const rows=state.bvs.filter(b=>b.adresse?.departement===dept);return `<section class="panel report-section"><div class="panel-head"><div><h3>${escapeHtml(dept)}</h3><p>${rows.length} bureau(x)</p></div><span class="badge success">Département</span></div><div class="table-panel"><table class="data-table"><thead><tr><th>ID_BV</th><th>Adresse</th><th>Président</th><th>Vice-président</th><th>Secrétaire</th></tr></thead><tbody>${rows.map(b=>`<tr><td class="code">${escapeHtml(b.idBV)}</td><td>${b.adresse.noRue}, ${escapeHtml(b.adresse.rue)}</td><td>${escapeHtml(responsibleName(b.responsables.president))}</td><td>${escapeHtml(responsibleName(b.responsables.vice))}</td><td>${escapeHtml(responsibleName(b.responsables.secretaire))}</td></tr>`).join('')}</tbody></table></div></section>`}).join('');}
function fillElectByDept(target){const depts=countByDept(state.electeurs,e=>e.adresse?.departement);if(!depts.length){target.innerHTML=`<section class="panel"><div class="empty-state"><strong>Aucun électeur</strong>Enregistrez des électeurs pour obtenir le regroupement demandé.</div></section>`;return;}target.innerHTML=depts.map(([dept])=>{const bvIds=[...new Set(state.electeurs.filter(e=>e.adresse?.departement===dept).map(e=>e.idBV))];return `<section class="panel report-section"><div class="panel-head"><div><h3>${escapeHtml(dept)}</h3><p>Électeurs groupés par bureau de vote</p></div><span class="badge">${state.electeurs.filter(e=>e.adresse?.departement===dept).length} électeur(s)</span></div><div class="panel-body">${bvIds.map(id=>{const rows=state.electeurs.filter(e=>e.adresse?.departement===dept&&e.idBV===id);return `<div class="group-title">Bureau ${escapeHtml(id)}</div><div class="table-panel"><table class="data-table"><thead><tr><th>NINU</th><th>Nom</th><th>Prénom</th><th>Adresse</th></tr></thead><tbody>${rows.map(e=>`<tr><td class="code">${escapeHtml(e.ninu)}</td><td>${escapeHtml(e.nom)}</td><td>${escapeHtml(e.prenom)}</td><td>${e.adresse.noRue}, ${escapeHtml(e.adresse.rue)}</td></tr>`).join('')}</tbody></table></div>`}).join('')||'<div class="empty-state">Aucun BV trouvé dans ce regroupement.</div>'}</div></section>`}).join('');}
function fillResults(target){const total=state.votes.length,blancs=state.votes.filter(v=>v.idCandidat==='BLANC').length,nonBlancs=total-blancs;const counts=state.candidats.map(c=>({...c,totalVotes:state.votes.filter(v=>v.idCandidat===c.idCandidat).length})).filter(c=>c.totalVotes>0).sort((a,b)=>b.totalVotes-a.totalVotes);const top=counts.length?counts[0].totalVotes:1;const depts=countByDept(state.bvs,b=>b.adresse?.departement);target.innerHTML=`<section class="summary-grid"><div class="summary-card"><span>Total votes</span><strong>${total}</strong></div><div class="summary-card"><span>Votes non blancs</span><strong>${nonBlancs}</strong></div><div class="summary-card"><span>Votes blancs</span><strong>${blancs}</strong></div><div class="summary-card"><span>Candidats votés</span><strong>${counts.length}</strong></div></section><section class="panel report-section" style="margin-top:18px"><div class="panel-head"><div><h3>Résumé national</h3><p>Le pourcentage est calculé sur le total des votes enregistrés, comme dans le module C.</p></div></div><div class="panel-body"><div class="result-bars">${counts.length?counts.map(c=>{const pct=total?c.totalVotes/total*100:0;return `<div class="result-row"><div class="result-label"><strong>${escapeHtml(c.nom)} ${escapeHtml(c.prenom)}</strong><span>${escapeHtml(c.idCandidat)} · ${c.totalVotes} vote(s)</span></div><div class="result-track"><span style="width:${Math.max(3,pct)}%"></span></div><div class="result-pct">${pct.toFixed(2)}%</div></div>`}).join(''):`<div class="empty-state"><strong>Pas encore de résultat</strong>Les votes enregistrés apparaîtront ici.</div>`}</div></div></section><section class="panel report-section"><div class="panel-head"><div><h3>Situation par département</h3><p>Volumes de votes rattachés aux BV du département.</p></div></div><div class="panel-body">${depts.length?depts.map(([dept])=>{const ids=state.bvs.filter(b=>b.adresse?.departement===dept).map(b=>b.idBV);const votes=state.votes.filter(v=>ids.includes(v.idBV));const pct=total?votes.length/total*100:0;return `<div class="bar-row" style="margin-bottom:14px"><span class="label">${escapeHtml(dept)}</span><div class="bar"><span style="width:${Math.max(votes.length?4:0,pct)}%"></span></div><span class="bar-value">${votes.length}</span></div>`}).join(''):`<div class="empty-state"><strong>Aucun département</strong></div>`}</div></section>`;}

function exportJSON(){const blob=new Blob([JSON.stringify(state,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=`election-ueh-${todayInput()}.json`;a.click();URL.revokeObjectURL(a.href);notify('Export JSON créé.');}
document.getElementById('importInput').addEventListener('change', e=>{const file=e.target.files?.[0];if(!file)return;const reader=new FileReader();reader.onload=()=>{try{const imported=JSON.parse(reader.result);state=Object.assign(emptyState(),imported);saveState();notify('Données importées.');render();}catch(err){notify('Fichier JSON invalide.','error');}};reader.readAsText(file);e.target.value='';});

function openGlobalSearch(){
  const overlay=document.createElement('div');overlay.className='backdrop-search';overlay.id='globalSearchOverlay';overlay.innerHTML=`<div class="global-search"><input autofocus id="globalSearchInput" placeholder="Rechercher dans les BV, électeurs, candidats, votes…"><div class="search-results" id="globalSearchResults"></div><div class="responsive-note" style="padding:10px 8px 0">Astuce : Ctrl/Cmd + K ouvre cette recherche.</div></div>`;document.body.appendChild(overlay);const input=overlay.querySelector('input');const results=overlay.querySelector('#globalSearchResults');function refresh(){const q=input.value.toLowerCase().trim();const all=[...state.bvs.map(x=>({kind:'BV',id:x.idBV,label:x.adresse?.departement||'',view:'bvs'})),...state.electeurs.map(x=>({kind:'Électeur',id:x.ninu,label:`${x.nom} ${x.prenom}`,view:'electeurs'})),...state.candidats.map(x=>({kind:'Candidat',id:x.idCandidat,label:`${x.nom} ${x.prenom}`,view:'candidats'})),...state.votes.map(x=>({kind:'Vote',id:x.idVote,label:`${x.ninu} · ${x.idBV}`,view:'votes'})),...state.partis.map(x=>({kind:'Parti',id:x.idPP,label:x.nom,view:'partis'}))];const rows=all.filter(x=>!q||`${x.kind} ${x.id} ${x.label}`.toLowerCase().includes(q)).slice(0,25);results.innerHTML=rows.map(x=>`<div class="search-result" data-view-search="${x.view}"><div><strong>${escapeHtml(x.id)}</strong><div><span>${escapeHtml(x.label)}</span></div></div><span>${escapeHtml(x.kind)}</span></div>`).join('')||`<div class="empty-state">Aucun résultat.</div>`;results.querySelectorAll('[data-view-search]').forEach(r=>r.addEventListener('click',()=>{setView(r.dataset.viewSearch);overlay.remove();}));}refresh();input.addEventListener('input',refresh);overlay.addEventListener('click',ev=>{if(ev.target===overlay)overlay.remove();});}

function demoData(){
  const bvs=[
    {idBV:'BV000001',adresse:{noRue:18,rue:'Rue Capois',departement:'Ouest'},responsables:{president:'1000000001',vice:'1000000002',secretaire:'1000000003'},membresAffectes:true},
    {idBV:'BV000002',adresse:{noRue:44,rue:'Rue Derenoncourt',departement:'Nord'},responsables:{president:'1000000004',vice:'1000000005',secretaire:'1000000006'},membresAffectes:true},
    {idBV:'BV000003',adresse:{noRue:9,rue:'Avenue Lamartiniere',departement:'Artibonite'},responsables:{president:'1000000007',vice:'1000000008',secretaire:'1000000009'},membresAffectes:true}
  ];
  const names=[['Dorsainvil','Mika','1000000001','18'],['Pierre','Natacha','1000000002','22'],['Jean','Samuel','1000000003','31'],['Lubin','Elie','1000000004','12'],['Charles','Marlene','1000000005','28'],['Joseph','Rico','1000000006','37'],['Cineas','Judith','1000000007','7'],['Louis','Marc','1000000008','15'],['Noel','Sarah','1000000009','24'],['Augustin','David','2000000010','30'],['Benoit','Diana','2000000011','13'],['Paul','Kevin','2000000012','5']];
  const electeurs=names.map((n,i)=>({ninu:n[2],nom:n[0],prenom:n[1],dateNaissance:`${1990+(i%8)}-0${(i%8)+1}-1${(i%8)}`,adresse:{noRue:Number(n[3]),rue:'Rue des Sciences',departement:i<3?'Ouest':i<6?'Nord':'Artibonite'},idBV:i<3?'BV000001':i<6?'BV000002':'BV000003'}));
  const candidats=[
    {idCandidat:'CA000001',ninu:'3000000001',nom:'Exemple',prenom:'Nadia',dateNaissance:'1988-02-12',adresse:{noRue:11,rue:'Rue du Centre',departement:'Ouest'},email:'nadia.exemple@gmail.com',telephone:'37000001',type:'0',dateInscription:todayInput()},
    {idCandidat:'CA000002',ninu:'3000000002',nom:'Renaud',prenom:'Patrick',dateNaissance:'1983-06-21',adresse:{noRue:23,rue:'Rue Lamartine',departement:'Nord'},email:'patrick.renaud@gmail.com',telephone:'37000002',type:'0',dateInscription:todayInput()},
    {idCandidat:'CA000003',ninu:'3000000003',nom:'Pierre-Louis',prenom:'Sandra',dateNaissance:'1987-09-14',adresse:{noRue:7,rue:'Rue des Arts',departement:'Artibonite'},email:'sandra.pl@gmail.com',telephone:'37000003',type:'0',dateInscription:todayInput()}
  ];
  const votes=[];let v=1;for(const [n,c,count] of [['1000000010','CA000001',1],['1000000011','CA000002',1],['1000000012','CA000003',1]]){if(state.electeurs.find(e=>e.ninu===n)){} }
  const eligible=electeurs.slice(0,8);const choices=['CA000001','CA000001','CA000002','CA000002','CA000003','CA000001','BLANC','CA000003'];eligible.forEach((e,i)=>votes.push({idVote:`VT${String(v++).padStart(6,'0')}`,dateVote:todayInput(),idBV:e.idBV,ninu:e.ninu,idCandidat:choices[i]}));
  const partis=[{idPP:'PP000001',nom:'Parti Démonstration',dateCreation:'2018-03-10',dateInscription:'2019-04-15',adresse:{noRue:5,rue:'Rue de la Paix',departement:'Ouest'},responsable:'Responsable Démo',telephone:'39000001',email:'parti.demo@gmail.com'}];
  return {partis,bvs,electeurs,candidats,votes};
}

window.closeModal=closeModal;window.setView=setView;window.openBVModal=openBVModal;window.viewBV=viewBV;window.deleteBV=deleteBV;window.openElecteurModal=openElecteurModal;window.viewElecteur=viewElecteur;window.deleteElecteur=deleteElecteur;window.openCandidatModal=openCandidatModal;window.deleteCandidat=deleteCandidat;window.openVoteModal=openVoteModal;window.openPartyModal=openPartyModal;window.deleteParty=deleteParty;window.setReportTab=setReportTab;window.exportJSON=exportJSON;

render();
