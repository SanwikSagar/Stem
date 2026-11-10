// STEM VCS - Professional GUI Application
// Fully integrated with STEM VCS backend

class STEMApp {
    constructor() {
        this.currentView = 'overview';
        this.selectedFile = null;
        this.selectedCommit = null;
        this.repoPath = null;
        this.repositories = this.loadRepositoriesList();
        this.currentRepoId = this.getCurrentRepoId();
        this.loading = false; // Add loading state
        this.state = {
            branch: 'main',
            files: [],
            commits: [],
            branches: [],
            staged: [],
            untracked: [],
            modified: [],
            remote: null,
            status: 'clean',
            tags: [],
            config: {}
        };
        
        this.init();
    }
    
    loadRepositoriesList() {
        const stored = localStorage.getItem('stem_repositories');
        if (stored) {
            try {
                return JSON.parse(stored);
            } catch (e) {
                return [];
            }
        }
        return [];
    }
    
    saveRepositoriesList() {
        localStorage.setItem('stem_repositories', JSON.stringify(this.repositories));
    }
    
    getCurrentRepoId() {
        const stored = localStorage.getItem('stem_current_repo');
        return stored || 'current';
    }
    
    setCurrentRepo(repoId) {
        this.currentRepoId = repoId;
        localStorage.setItem('stem_current_repo', repoId);
    }
    
    async init() {
        this.setupEventListeners();
        await this.loadRepository();
        this.render();
        this.startAutoRefresh();
    }
    
    setupEventListeners() {
        // Navigation
        document.querySelectorAll('.nav-item').forEach(item => {
            item.addEventListener('click', (e) => {
                const view = e.currentTarget.dataset.view;
                this.switchView(view);
            });
        });
        
        // Titlebar actions
        document.getElementById('pullBtn')?.addEventListener('click', () => this.pull());
        document.getElementById('pushBtn')?.addEventListener('click', () => this.push());
        document.getElementById('syncBtn')?.addEventListener('click', () => this.sync());
        document.getElementById('createBranchBtn')?.addEventListener('click', () => this.createBranch());
        document.getElementById('mergeBtn')?.addEventListener('click', () => this.merge());
        document.getElementById('addRepoBtn')?.addEventListener('click', () => this.showAddRepoDialog());
        
        // Repository selector
        document.getElementById('repoSelect')?.addEventListener('change', (e) => {
            this.switchRepository(e.target.value);
        });
        
        // Branch selector
        document.getElementById('branchSelect')?.addEventListener('change', (e) => {
            this.switchBranch(e.target.value);
        });
        
        // Right panel actions
        document.getElementById('stageAllBtn')?.addEventListener('click', () => this.stageAll());
        document.getElementById('commitBtn')?.addEventListener('click', () => this.showCommitDialog());
        document.getElementById('pullActionBtn')?.addEventListener('click', () => this.pull());
        document.getElementById('pushActionBtn')?.addEventListener('click', () => this.push());
        document.getElementById('syncActionBtn')?.addEventListener('click', () => this.sync());
        
        // Remote change button
        document.getElementById('changeRemoteBtn')?.addEventListener('click', () => this.changeRemote());
        
        // Search
        document.getElementById('searchInput')?.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                this.search(e.target.value);
            }
        });
        
        // Modal keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            // Global shortcuts
            if (e.ctrlKey && e.key === 'k') {
                e.preventDefault();
                document.getElementById('searchInput')?.focus();
            }
            
            // Escape to close modals
            if (e.key === 'Escape') {
                this.closeRemoteModal();
                this.closeCommitModal();
                this.closeBranchModal();
                this.closeTagModal();
                this.closeConfirmModal();
                this.closeAddRepoModal();
            }
        });
        
        // Remote modal - Enter to submit
        document.getElementById('remoteUrlInput')?.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                e.preventDefault();
                this.saveRemoteUrl();
            }
        });
        
        // Commit modal - Ctrl+Enter to submit
        document.getElementById('commitMessageInput')?.addEventListener('keydown', (e) => {
            if (e.ctrlKey && e.key === 'Enter') {
                e.preventDefault();
                this.saveCommit();
            }
        });
        
        // Branch modal - Enter to submit
        document.getElementById('branchNameInput')?.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                e.preventDefault();
                this.saveBranch();
            }
        });
        
        // Tag modal - Enter to submit
        document.getElementById('tagNameInput')?.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                e.preventDefault();
                this.saveTag();
            }
        });
    }
    
    async loadRepository() {
        try {
            // Load status
            const statusResp = await this.apiCall('/api/status');
            if (statusResp) {
                this.state.branch = statusResp.branch || 'main';
                this.state.status = statusResp.status || 'clean';
            }
            
            // Load files
            const filesResp = await this.apiCall('/api/files');
            if (filesResp && Array.isArray(filesResp)) {
                this.state.files = filesResp;
                this.categorizeFiles();
            }
            
            // Load commits
            const historyResp = await this.apiCall('/api/history');
            if (historyResp && Array.isArray(historyResp)) {
                this.state.commits = historyResp;
            }
            
            // Load branches
            const branchesResp = await this.apiCall('/api/branches');
            if (branchesResp && Array.isArray(branchesResp)) {
                this.state.branches = branchesResp;
            }
            
            // Load remote
            const remoteResp = await this.apiCall('/api/remote');
            if (remoteResp) {
                this.state.remote = remoteResp;
            }
            
            // Load tags
            const tagsResp = await this.apiCall('/api/tags');
            if (tagsResp && Array.isArray(tagsResp)) {
                this.state.tags = tagsResp;
            }
            
            // Load config
            const configResp = await this.apiCall('/api/config');
            if (configResp) {
                this.state.config = configResp;
            }
            
            this.setStatus('Repository loaded');
        } catch (error) {
            console.error('Failed to load repository:', error);
            this.showDemoData();
            this.setStatus('Using demo data');
        }
    }
    
    categorizeFiles() {
        this.state.staged = this.state.files.filter(f => f.staged);
        this.state.modified = this.state.files.filter(f => f.status === 'modified');
        this.state.untracked = this.state.files.filter(f => f.status === 'untracked');
    }
    
    async apiCall(endpoint, options = {}) {
        try {
            const response = await fetch(endpoint, {
                headers: { 'Content-Type': 'application/json' },
                ...options
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error(`API call failed: ${endpoint}`, error);
            return null;
        }
    }
    
    showDemoData() {
        this.state.branch = 'main';
        this.state.files = [
            { path: 'src/renderer/Renderer.cpp', status: 'modified', staged: true, additions: 102, deletions: 18 },
            { path: 'src/renderer/Shader.cpp', status: 'modified', staged: true, additions: 28, deletions: 6 },
            { path: 'include/Renderer.h', status: 'modified', staged: true, additions: 12, deletions: 3 },
            { path: 'README.md', status: 'modified', staged: false, additions: 5, deletions: 2 },
            { path: 'CMakeLists.txt', status: 'modified', staged: false, additions: 3, deletions: 1 }
        ];
        
        this.state.commits = [
            { hash: 'a1b2c3d', message: 'Improve renderer performance', author: 'You', date: '2 hours ago', files: 3, additions: 142, deletions: 27 },
            { hash: 'd4e5f6g', message: 'Refactor memory allocator', author: 'You', date: '1 day ago', files: 5, additions: 85, deletions: 42 },
            { hash: 'h7i8j9k', message: 'Add Vulkan backend', author: 'Alex', date: '2 days ago', files: 12, additions: 320, deletions: 15 },
            { hash: 'l1m2n3o', message: 'Fix asset import crash', author: 'You', date: '3 days ago', files: 2, additions: 15, deletions: 8 },
            { hash: 'p4q5r6s', message: 'Improve logging system', author: 'Sam', date: '5 days ago', files: 4, additions: 95, deletions: 30 },
            { hash: 't7u8v9w', message: 'Initial commit', author: 'You', date: '1 week ago', files: 45, additions: 2500, deletions: 0 }
        ];
        
        this.state.branches = [
            { name: 'main', current: true, ahead: 2, behind: 0 },
            { name: 'feature-vulkan', current: false, ahead: 5, behind: 1 },
            { name: 'bugfix-memory', current: false, ahead: 0, behind: 3 }
        ];
        
        this.state.remote = {
            name: 'Google Drive',
            url: 'https://drive.google.com/drive/folders/1a2b3c4d5e6f',
            connected: true
            // No size/total/percent - will not be displayed
        };
        
        this.categorizeFiles();
    }
    
    switchView(view) {
        this.currentView = view;
        
        // Update navigation
        document.querySelectorAll('.nav-item').forEach(item => {
            item.classList.toggle('active', item.dataset.view === view);
        });
        
        this.render();
    }
    
    render() {
        this.updateStatusBar();
        this.updateRightPanel();
        this.updateRepositorySelector();
        this.renderCurrentView();
    }
    
    updateStatusBar() {
        document.getElementById('statusBarBranch').textContent = this.state.branch;
        document.getElementById('statusAhead').textContent = '0';
        document.getElementById('statusBehind').textContent = '0';
        
        const changesText = this.state.modified.length > 0 
            ? `${this.state.modified.length} files changed` 
            : 'Working Tree Clean';
        document.getElementById('statusChanges').innerHTML = `
            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
            <span>${changesText}</span>
        `;
    }
    
    updateRightPanel() {
        // Update branch selector
        const branchSelect = document.getElementById('branchSelect');
        if (branchSelect) {
            branchSelect.innerHTML = this.state.branches.map(b => 
                `<option value="${b.name}" ${b.current ? 'selected' : ''}>${b.name}</option>`
            ).join('');
        }
        
        // Update status
        document.getElementById('statusBranch').textContent = this.state.branch;
        document.getElementById('statusStaged').textContent = this.state.staged.length;
        document.getElementById('statusWorking').textContent = this.state.modified.length > 0 ? `${this.state.modified.length} changed` : 'Clean';
        document.getElementById('statusUntracked').textContent = this.state.untracked.length;
        
        // Update changes badge
        const badge = document.getElementById('changesBadge');
        if (badge) {
            const totalChanges = this.state.modified.length + this.state.untracked.length;
            badge.textContent = totalChanges;
            badge.style.display = totalChanges > 0 ? 'block' : 'none';
        }
        
        // Update remote info
        if (this.state.remote && this.state.remote.url) {
            document.getElementById('remoteName').textContent = this.state.remote.name || 'Google Drive';
            document.getElementById('remoteStatus').textContent = this.state.remote.connected ? 'Connected' : 'Disconnected';
            const remoteUrlEl = document.getElementById('remoteUrl');
            if (remoteUrlEl) {
                remoteUrlEl.textContent = this.state.remote.url;
                remoteUrlEl.title = this.state.remote.url; // Show full URL on hover
            }
            
            // Only show storage stats if available from server
            const remoteSizeEl = document.getElementById('remoteSize');
            if (remoteSizeEl) {
                if (this.state.remote.size && this.state.remote.total) {
                    remoteSizeEl.textContent = `${this.state.remote.size} / ${this.state.remote.total} (${this.state.remote.percent || 0}%)`;
                    remoteSizeEl.style.display = 'block';
                } else {
                    remoteSizeEl.style.display = 'none';
                }
            }
        } else {
            document.getElementById('remoteName').textContent = 'Not configured';
            document.getElementById('remoteStatus').textContent = 'Disconnected';
            const remoteUrlEl = document.getElementById('remoteUrl');
            if (remoteUrlEl) {
                remoteUrlEl.textContent = 'No remote set';
            }
            const remoteSizeEl = document.getElementById('remoteSize');
            if (remoteSizeEl) {
                remoteSizeEl.style.display = 'none';
            }
        }
        
        // Update activity list
        this.renderActivityList();
        
        // Update folder tree
        this.renderFolderTree();
    }
    
    renderCurrentView() {
        const content = document.getElementById('mainContent');
        
        switch(this.currentView) {
            case 'overview':
                content.innerHTML = this.renderOverview();
                break;
            case 'changes':
                content.innerHTML = this.renderChanges();
                this.attachFileListeners();
                break;
            case 'history':
                content.innerHTML = this.renderHistory();
                this.attachCommitListeners();
                break;
            case 'branches':
                content.innerHTML = this.renderBranches();
                break;
            case 'tags':
                content.innerHTML = this.renderTags();
                break;
            case 'remotes':
                content.innerHTML = this.renderRemotes();
                break;
            case 'repositories':
                content.innerHTML = this.renderRepositories();
                this.attachRepositoryListeners();
                break;
            case 'settings':
                content.innerHTML = this.renderSettings();
                this.attachSettingsListeners();
                break;
        }
    }
    
    renderOverview() {
        return `
            <div class="view-container">
                <div class="view-header">
                    <h1 class="view-title">Overview</h1>
                    <p class="view-subtitle">Repository dashboard</p>
                </div>
                
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 16px; margin-bottom: 24px;">
                    <div style="background: var(--vscode-hover); padding: 16px; border-radius: 4px; border: 1px solid var(--vscode-border);">
                        <div style="font-size: 11px; color: var(--text-secondary); margin-bottom: 8px;">STAGED FILES</div>
                        <div style="font-size: 24px; font-weight: 600; color: var(--accent-green);">${this.state.staged.length}</div>
                    </div>
                    <div style="background: var(--vscode-hover); padding: 16px; border-radius: 4px; border: 1px solid var(--vscode-border);">
                        <div style="font-size: 11px; color: var(--text-secondary); margin-bottom: 8px;">MODIFIED FILES</div>
                        <div style="font-size: 24px; font-weight: 600; color: var(--accent-yellow);">${this.state.modified.length}</div>
                    </div>
                    <div style="background: var(--vscode-hover); padding: 16px; border-radius: 4px; border: 1px solid var(--vscode-border);">
                        <div style="font-size: 11px; color: var(--text-secondary); margin-bottom: 8px;">TOTAL COMMITS</div>
                        <div style="font-size: 24px; font-weight: 600; color: var(--accent-blue);">${this.state.commits.length}</div>
                    </div>
                </div>
                
                <h2 style="font-size: 14px; font-weight: 600; margin-bottom: 12px; color: var(--text-bright);">Recent Commits</h2>
                <div class="commit-list">
                    ${this.state.commits.slice(0, 5).map(commit => `
                        <div class="commit-item">
                            <div class="commit-indicator"></div>
                            <div class="commit-content">
                                <div class="commit-message">${commit.message}</div>
                                <div class="commit-meta">${commit.author} • ${commit.date} • ${commit.hash}</div>
                            </div>
                        </div>
                    `).join('')}
                </div>
            </div>
        `;
    }
    
    renderChanges() {
        const allFiles = [...this.state.staged, ...this.state.modified, ...this.state.untracked];
        
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Changes</h1>
                        <p class="view-subtitle">${allFiles.length} files • ${this.state.staged.length} staged</p>
                    </div>
                    <div style="display: flex; gap: 8px;">
                        <button class="btn-secondary" onclick="window.stemApp.stageAll()">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                            Stage All
                        </button>
                        <button class="btn-secondary" onclick="window.stemApp.unstageAll()">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M2.75 8a.75.75 0 000 1.5h10.5a.75.75 0 000-1.5H2.75z"/></svg>
                            Unstage All
                        </button>
                        <button class="btn-primary" onclick="window.stemApp.showCommitDialog()">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M1.643 3.143L.427 1.927A.25.25 0 000 2.104V5.75c0 .138.112.25.25.25h3.646a.25.25 0 00.177-.427L2.715 4.215a6.5 6.5 0 11-1.18 4.458.75.75 0 10-1.493.154 8.001 8.001 0 101.6-5.684zM7.75 4a.75.75 0 01.75.75v2.992l2.028.812a.75.75 0 01-.557 1.392l-2.5-1A.75.75 0 017 8.25v-3.5A.75.75 0 017.75 4z"/></svg>
                            Commit
                        </button>
                    </div>
                </div>
                
                ${this.state.staged.length > 0 ? `
                    <div class="file-section">
                        <div class="section-header">
                            <h3 class="section-title">
                                <svg width="16" height="16" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                                Staged Changes
                                <span class="section-count">${this.state.staged.length}</span>
                            </h3>
                        </div>
                        <div class="file-list-modern">
                            ${this.state.staged.map((file, idx) => `
                                <div class="file-item-modern" data-file-idx="${idx}">
                                    <div class="file-icon">
                                        ${this.getFileIcon(file.path)}
                                    </div>
                                    <div class="file-info">
                                        <div class="file-path">${file.path}</div>
                                        <div class="file-meta">
                                            <span class="file-status status-staged">Staged</span>
                                            ${file.size ? `<span class="file-size">${this.formatFileSize(file.size)}</span>` : ''}
                                        </div>
                                    </div>
                                    <div class="file-stats">
                                        ${file.additions ? `<span class="additions">+${file.additions}</span>` : ''}
                                        ${file.deletions ? `<span class="deletions">-${file.deletions}</span>` : ''}
                                    </div>
                                </div>
                            `).join('')}
                        </div>
                    </div>
                ` : ''}
                
                ${this.state.modified.length > 0 ? `
                    <div class="file-section">
                        <div class="section-header">
                            <h3 class="section-title">
                                <svg width="16" height="16" viewBox="0 0 16 16"><path fill="currentColor" d="M2.75 2.5a.25.25 0 00-.25.25v10.5c0 .138.112.25.25.25h10.5a.25.25 0 00.25-.25V2.75a.25.25 0 00-.25-.25H2.75zM1 2.75C1 1.784 1.784 1 2.75 1h10.5c.966 0 1.75.784 1.75 1.75v10.5A1.75 1.75 0 0113.25 15H2.75A1.75 1.75 0 011 13.25V2.75z"/></svg>
                                Modified Files
                                <span class="section-count">${this.state.modified.length}</span>
                            </h3>
                        </div>
                        <div class="file-list-modern">
                            ${this.state.modified.map((file, idx) => `
                                <div class="file-item-modern" data-file-idx="${idx + this.state.staged.length}">
                                    <div class="file-icon">
                                        ${this.getFileIcon(file.path)}
                                    </div>
                                    <div class="file-info">
                                        <div class="file-path">${file.path}</div>
                                        <div class="file-meta">
                                            <span class="file-status status-modified">Modified</span>
                                            ${file.size ? `<span class="file-size">${this.formatFileSize(file.size)}</span>` : ''}
                                        </div>
                                    </div>
                                    <div class="file-stats">
                                        ${file.additions ? `<span class="additions">+${file.additions}</span>` : ''}
                                        ${file.deletions ? `<span class="deletions">-${file.deletions}</span>` : ''}
                                    </div>
                                </div>
                            `).join('')}
                        </div>
                    </div>
                ` : ''}
                
                ${this.state.untracked.length > 0 ? `
                    <div class="file-section">
                        <div class="section-header">
                            <h3 class="section-title">
                                <svg width="16" height="16" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                                Untracked Files
                                <span class="section-count">${this.state.untracked.length}</span>
                            </h3>
                        </div>
                        <div class="file-list-modern">
                            ${this.state.untracked.slice(0, 20).map((file, idx) => `
                                <div class="file-item-modern" data-file-idx="${idx + this.state.staged.length + this.state.modified.length}">
                                    <div class="file-icon">
                                        ${this.getFileIcon(file.path)}
                                    </div>
                                    <div class="file-info">
                                        <div class="file-path">${file.path}</div>
                                        <div class="file-meta">
                                            <span class="file-status status-untracked">Untracked</span>
                                            ${file.size ? `<span class="file-size">${this.formatFileSize(file.size)}</span>` : ''}
                                        </div>
                                    </div>
                                </div>
                            `).join('')}
                            ${this.state.untracked.length > 20 ? `
                                <div class="file-item-modern file-more">
                                    <span>...and ${this.state.untracked.length - 20} more files</span>
                                </div>
                            ` : ''}
                        </div>
                    </div>
                ` : ''}
                
                ${allFiles.length === 0 ? `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                        <h3>Working tree clean</h3>
                        <p>No changes to commit</p>
                    </div>
                ` : ''}
            </div>
        `;
    }
    
    getFileIcon(path) {
        const ext = path.split('.').pop().toLowerCase();
        const icons = {
            'js': '📄', 'ts': '📘', 'jsx': '⚛️', 'tsx': '⚛️',
            'html': '🌐', 'css': '🎨', 'scss': '🎨', 'sass': '🎨',
            'json': '📋', 'md': '📝', 'txt': '📄',
            'py': '🐍', 'java': '☕', 'c': '©️', 'cpp': '©️', 'h': '📋',
            'png': '🖼️', 'jpg': '🖼️', 'jpeg': '🖼️', 'gif': '🖼️', 'svg': '🎨',
            'pdf': '📕', 'doc': '📘', 'docx': '📘', 'xls': '📗', 'xlsx': '📗'
        };
        return icons[ext] || '📄';
    }
    
    formatFileSize(bytes) {
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
        return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
    }
    
    renderHistory() {
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Commit History</h1>
                        <p class="view-subtitle">${this.state.commits.length} commits on ${this.state.branch}</p>
                    </div>
                    <div style="display: flex; gap: 8px;">
                        <button class="btn-secondary">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                            Filter
                        </button>
                        <button class="btn-secondary">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M11.5 7a4.5 4.5 0 11-9 0 4.5 4.5 0 019 0zm-.82 4.74a6 6 0 111.06-1.06l3.04 3.04a.75.75 0 11-1.06 1.06l-3.04-3.04z"/></svg>
                            Search commits
                        </button>
                    </div>
                </div>
                
                <div class="commit-graph">
                    ${this.state.commits.map((commit, index) => `
                        <div class="commit-row" data-commit="${commit.hash}">
                            <div class="commit-graph-line">
                                <svg width="40" height="100%" class="graph-svg">
                                    ${index > 0 ? '<line x1="20" y1="0" x2="20" y2="32" stroke="var(--accent-blue)" stroke-width="2"/>' : ''}
                                    <circle cx="20" cy="32" r="6" fill="var(--accent-blue)" stroke="var(--vscode-bg)" stroke-width="2"/>
                                    ${index < this.state.commits.length - 1 ? '<line x1="20" y1="32" x2="20" y2="100%" stroke="var(--accent-blue)" stroke-width="2"/>' : ''}
                                </svg>
                            </div>
                            <div class="commit-details">
                                <div class="commit-header-row">
                                    <div class="commit-message-main">${commit.message}</div>
                                    <div class="commit-actions">
                                        <button class="btn-icon" title="Copy hash">
                                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M0 6.75C0 5.784.784 5 1.75 5h1.5a.75.75 0 010 1.5h-1.5a.25.25 0 00-.25.25v7.5c0 .138.112.25.25.25h7.5a.25.25 0 00.25-.25v-1.5a.75.75 0 011.5 0v1.5A1.75 1.75 0 019.25 16h-7.5A1.75 1.75 0 010 14.25v-7.5z"/><path fill="currentColor" d="M5 1.75C5 .784 5.784 0 6.75 0h7.5C15.216 0 16 .784 16 1.75v7.5A1.75 1.75 0 0114.25 11h-7.5A1.75 1.75 0 015 9.25v-7.5zm1.75-.25a.25.25 0 00-.25.25v7.5c0 .138.112.25.25.25h7.5a.25.25 0 00.25-.25v-7.5a.25.25 0 00-.25-.25h-7.5z"/></svg>
                                        </button>
                                        <button class="btn-icon" title="View files">
                                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M1.75 2.5a.25.25 0 00-.25.25v10.5c0 .138.112.25.25.25h12.5a.25.25 0 00.25-.25V2.75a.25.25 0 00-.25-.25H1.75zM0 2.75C0 1.784.784 1 1.75 1h12.5c.966 0 1.75.784 1.75 1.75v10.5A1.75 1.75 0 0114.25 15H1.75A1.75 1.75 0 010 13.25V2.75z"/></svg>
                                        </button>
                                    </div>
                                </div>
                                <div class="commit-meta-row">
                                    <span class="commit-hash">${commit.hash}</span>
                                    <span class="commit-author">
                                        <svg width="12" height="12" viewBox="0 0 16 16"><path fill="currentColor" d="M8 8a3 3 0 100-6 3 3 0 000 6zm2-3a2 2 0 11-4 0 2 2 0 014 0zm4 8c0 1-1 1-1 1H3s-1 0-1-1 1-4 6-4 6 3 6 4zm-1-.004c-.001-.246-.154-.986-.832-1.664C11.516 10.68 10.289 10 8 10c-2.29 0-3.516.68-4.168 1.332-.678.678-.83 1.418-.832 1.664h10z"/></svg>
                                        ${commit.author}
                                    </span>
                                    <span class="commit-date">
                                        <svg width="12" height="12" viewBox="0 0 16 16"><path fill="currentColor" d="M1.643 3.143L.427 1.927A.25.25 0 000 2.104V5.75c0 .138.112.25.25.25h3.646a.25.25 0 00.177-.427L2.715 4.215a6.5 6.5 0 11-1.18 4.458.75.75 0 10-1.493.154 8.001 8.001 0 101.6-5.684zM7.75 4a.75.75 0 01.75.75v2.992l2.028.812a.75.75 0 01-.557 1.392l-2.5-1A.75.75 0 017 8.25v-3.5A.75.75 0 017.75 4z"/></svg>
                                        ${commit.date}
                                    </span>
                                    <span class="commit-stats">
                                        <span class="stat-files">${commit.files} files</span>
                                        <span class="stat-additions">+${commit.additions || 0}</span>
                                        <span class="stat-deletions">-${commit.deletions || 0}</span>
                                    </span>
                                </div>
                            </div>
                        </div>
                    `).join('')}
                </div>
                
                ${this.state.commits.length === 0 ? `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M11.75 2.5a.75.75 0 100 1.5.75.75 0 000-1.5zm-2.25.75a2.25 2.25 0 113 2.122V6A2.5 2.5 0 0110 8.5H6a1 1 0 00-1 1v1.128a2.251 2.251 0 11-1.5 0V5.372a2.25 2.25 0 111.5 0v1.836A2.492 2.492 0 016 7h4a1 1 0 001-1v-.628A2.25 2.25 0 019.5 3.25zM4.25 12a.75.75 0 100 1.5.75.75 0 000-1.5zM3.5 3.25a.75.75 0 111.5 0 .75.75 0 01-1.5 0z"/></svg>
                        <h3>No commits yet</h3>
                        <p>Start by staging some files and creating your first commit</p>
                    </div>
                ` : ''}
            </div>
        `;
    }
    
    renderBranches() {
        const currentBranch = this.state.branch;
        
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Branches</h1>
                        <p class="view-subtitle">${this.state.branches.length} branches • Current: ${currentBranch}</p>
                    </div>
                    <button class="btn-primary" onclick="window.stemApp.createBranch()">
                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                        New Branch
                    </button>
                </div>
                
                <div class="branch-list-modern">
                    ${this.state.branches.map(branch => `
                        <div class="branch-item-modern ${branch.current ? 'branch-current' : ''}" data-branch="${branch.name}">
                            <div class="branch-icon">
                                <svg width="16" height="16" viewBox="0 0 16 16"><path fill="${branch.current ? 'var(--accent-blue)' : 'var(--text-secondary)'}" d="M11.75 2.5a.75.75 0 100 1.5.75.75 0 000-1.5zm-2.25.75a2.25 2.25 0 113 2.122V6A2.5 2.5 0 0110 8.5H6a1 1 0 00-1 1v1.128a2.251 2.251 0 11-1.5 0V5.372a2.25 2.25 0 111.5 0v1.836A2.492 2.492 0 016 7h4a1 1 0 001-1v-.628A2.25 2.25 0 019.5 3.25zM4.25 12a.75.75 0 100 1.5.75.75 0 000-1.5zM3.5 3.25a.75.75 0 111.5 0 .75.75 0 01-1.5 0z"/></svg>
                            </div>
                            <div class="branch-info">
                                <div class="branch-name-row">
                                    <span class="branch-name-text">${branch.name}</span>
                                    ${branch.current ? '<span class="branch-badge current">CURRENT</span>' : ''}
                                </div>
                                <div class="branch-meta">
                                    ${branch.ahead > 0 ? `<span class="branch-stat ahead">${branch.ahead} ahead</span>` : ''}
                                    ${branch.behind > 0 ? `<span class="branch-stat behind">${branch.behind} behind</span>` : ''}
                                </div>
                            </div>
                            <div class="branch-actions">
                                ${!branch.current ? `
                                    <button class="btn-branch-action" onclick="window.stemApp.switchBranch('${branch.name}')" title="Switch to this branch">
                                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                                        Checkout
                                    </button>
                                    <button class="btn-branch-action" onclick="window.stemApp.mergeBranch('${branch.name}')" title="Merge into current branch">
                                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M5.45 5.154A4.25 4.25 0 009.25 7.5h1.378a2.251 2.251 0 110 1.5H9.25A5.734 5.734 0 015 7.123v3.505a2.25 2.25 0 11-1.5 0V5.372a2.25 2.25 0 111.95-.218zM4.25 3.5a.75.75 0 100-1.5.75.75 0 000 1.5zm8.5 9.5a.75.75 0 100-1.5.75.75 0 000 1.5zM5 11.25a.75.75 0 11-1.5 0 .75.75 0 011.5 0z"/></svg>
                                        Merge
                                    </button>
                                    <button class="btn-branch-action danger" onclick="window.stemApp.deleteBranch('${branch.name}')" title="Delete this branch">
                                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M11 1.75V3h2.25a.75.75 0 010 1.5H2.75a.75.75 0 010-1.5H5V1.75C5 .784 5.784 0 6.75 0h2.5C10.216 0 11 .784 11 1.75zM4.496 6.675a.75.75 0 10-1.492.15l.66 6.6A1.75 1.75 0 005.405 15h5.19c.9 0 1.652-.681 1.741-1.576l.66-6.6a.75.75 0 00-1.492-.149l-.66 6.6a.25.25 0 01-.249.225h-5.19a.25.25 0 01-.249-.225l-.66-6.6z"/></svg>
                                        Delete
                                    </button>
                                ` : '<span class="branch-current-label">Active branch</span>'}
                            </div>
                        </div>
                    `).join('')}
                </div>
                
                ${this.state.branches.length === 0 ? `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M11.75 2.5a.75.75 0 100 1.5.75.75 0 000-1.5zm-2.25.75a2.25 2.25 0 113 2.122V6A2.5 2.5 0 0110 8.5H6a1 1 0 00-1 1v1.128a2.251 2.251 0 11-1.5 0V5.372a2.25 2.25 0 111.5 0v1.836A2.492 2.492 0 016 7h4a1 1 0 001-1v-.628A2.25 2.25 0 019.5 3.25zM4.25 12a.75.75 0 100 1.5.75.75 0 000-1.5zM3.5 3.25a.75.75 0 111.5 0 .75.75 0 01-1.5 0z"/></svg>
                        <h3>No branches yet</h3>
                        <p>Create your first branch to get started</p>
                    </div>
                ` : ''}
            </div>
        `;
    }
    
    renderTags() {
        const tags = this.state.tags || [];
        
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Tags</h1>
                        <p class="view-subtitle">${tags.length} tags</p>
                    </div>
                    <button class="btn-primary" onclick="window.stemApp.createTag()">
                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                        New Tag
                    </button>
                </div>
                
                ${tags.length > 0 ? `
                    <div class="tag-list-modern">
                        ${tags.map(tag => `
                            <div class="tag-item-modern">
                                <div class="tag-icon">
                                    <svg width="16" height="16" viewBox="0 0 16 16"><path fill="var(--accent-purple)" d="M2.5 7.775V2.75c0-.138.112-.25.25-.25h5.025a.25.25 0 01.177.073l6.25 6.25a.25.25 0 010 .354l-5.025 5.025a.25.25 0 01-.354 0l-6.25-6.25a.25.25 0 01-.073-.177zm-1.5 0V2.75C1 1.784 1.784 1 2.75 1h5.025c.464 0 .91.184 1.238.513l6.25 6.25a1.75 1.75 0 010 2.474l-5.026 5.026a1.75 1.75 0 01-2.474 0l-6.25-6.25A1.75 1.75 0 011 7.775zM6 5a1 1 0 100 2 1 1 0 000-2z"/></svg>
                                </div>
                                <div class="tag-info">
                                    <div class="tag-name">${tag.name}</div>
                                    <div class="tag-meta">${tag.commit} • ${tag.date}</div>
                                </div>
                                <button class="btn-branch-action danger" onclick="window.stemApp.deleteTag('${tag.name}')" title="Delete tag">
                                    <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M11 1.75V3h2.25a.75.75 0 010 1.5H2.75a.75.75 0 010-1.5H5V1.75C5 .784 5.784 0 6.75 0h2.5C10.216 0 11 .784 11 1.75zM4.496 6.675a.75.75 0 10-1.492.15l.66 6.6A1.75 1.75 0 005.405 15h5.19c.9 0 1.652-.681 1.741-1.576l.66-6.6a.75.75 0 00-1.492-.149l-.66 6.6a.25.25 0 01-.249.225h-5.19a.25.25 0 01-.249-.225l-.66-6.6z"/></svg>
                                    Delete
                                </button>
                            </div>
                        `).join('')}
                    </div>
                ` : `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M2.5 7.775V2.75c0-.138.112-.25.25-.25h5.025a.25.25 0 01.177.073l6.25 6.25a.25.25 0 010 .354l-5.025 5.025a.25.25 0 01-.354 0l-6.25-6.25a.25.25 0 01-.073-.177zm-1.5 0V2.75C1 1.784 1.784 1 2.75 1h5.025c.464 0 .91.184 1.238.513l6.25 6.25a1.75 1.75 0 010 2.474l-5.026 5.026a1.75 1.75 0 01-2.474 0l-6.25-6.25A1.75 1.75 0 011 7.775zM6 5a1 1 0 100 2 1 1 0 000-2z"/></svg>
                        <h3>No tags yet</h3>
                        <p>Create tags to mark important points in your history</p>
                    </div>
                `}
            </div>
        `;
    }
    
    renderRemotes() {
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Remote Storage</h1>
                        <p class="view-subtitle">Configure cloud storage</p>
                    </div>
                    <button class="btn-primary" onclick="window.stemApp.changeRemote()">
                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                        Configure Remote
                    </button>
                </div>
                
                ${this.state.remote && this.state.remote.url ? `
                    <div class="remote-card-large">
                        <div class="remote-header">
                            <div class="remote-icon-large">
                                <svg width="32" height="32" viewBox="0 0 16 16"><path fill="var(--accent-blue)" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                            </div>
                            <div>
                                <h3 style="font-size: 18px; font-weight: 600; color: var(--text-bright); margin: 0 0 4px 0;">${this.state.remote.name || 'Google Drive'}</h3>
                                <div class="remote-status">
                                    <span class="status-indicator ${this.state.remote.connected ? 'connected' : 'disconnected'}"></span>
                                    <span>${this.state.remote.connected ? 'Connected' : 'Disconnected'}</span>
                                </div>
                            </div>
                        </div>
                        <div class="remote-details">
                            <div class="detail-row">
                                <span class="detail-label">URL</span>
                                <span class="detail-value">${this.state.remote.url}</span>
                            </div>
                        </div>
                        <div class="remote-actions">
                            <button class="btn-secondary" onclick="window.stemApp.changeRemote()">Change URL</button>
                            <button class="btn-secondary" onclick="window.stemApp.pull()">Pull</button>
                            <button class="btn-primary" onclick="window.stemApp.push()">Push</button>
                        </div>
                    </div>
                ` : `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                        <h3>No remote configured</h3>
                        <p>Configure a remote storage location to sync your repository</p>
                        <button class="btn-primary" onclick="window.stemApp.changeRemote()" style="margin-top: 16px;">
                            Configure Remote
                        </button>
                    </div>
                `}
            </div>
        `;
    }
    
    renderRepositories() {
        return `
            <div class="view-container">
                <div class="view-header">
                    <div>
                        <h1 class="view-title">Repositories</h1>
                        <p class="view-subtitle">${this.repositories.length} repositories</p>
                    </div>
                    <button class="btn-primary" onclick="window.stemApp.showAddRepoDialog()">
                        <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                        Add Repository
                    </button>
                </div>
                
                ${this.repositories.length > 0 ? `
                    <div class="repository-list-modern">
                        ${this.repositories.map(repo => {
                            const isCurrent = repo.id === this.currentRepoId;
                            return `
                                <div class="repository-item-modern ${isCurrent ? 'repo-current' : ''}" data-repo-id="${repo.id}">
                                    <div class="repo-icon">
                                        <svg width="20" height="20" viewBox="0 0 16 16"><path fill="${isCurrent ? 'var(--accent-blue)' : 'var(--text-secondary)'}" d="M2 2.5A2.5 2.5 0 014.5 0h8.75a.75.75 0 01.75.75v12.5a.75.75 0 01-.75.75h-2.5a.75.75 0 110-1.5h1.75v-2h-8a1 1 0 00-.714 1.7.75.75 0 01-1.072 1.05A2.495 2.495 0 012 11.5v-9zm10.5-1V9h-8c-.356 0-.694.074-1 .208V2.5a1 1 0 011-1h8z"/></svg>
                                    </div>
                                    <div class="repo-info">
                                        <div class="repo-name-row">
                                            <span class="repo-name-text">${repo.name}</span>
                                            ${isCurrent ? '<span class="branch-badge current">CURRENT</span>' : ''}
                                        </div>
                                        <div class="repo-path">${repo.path}</div>
                                        ${repo.remote ? `<div class="repo-remote">📡 ${repo.remote.substring(0, 60)}${repo.remote.length > 60 ? '...' : ''}</div>` : '<div class="repo-remote" style="color: var(--text-dim);">No remote configured</div>'}
                                        <div class="repo-meta">Added ${new Date(repo.addedAt).toLocaleDateString()}</div>
                                    </div>
                                    <div class="repo-actions">
                                        ${!isCurrent ? `
                                            <button class="btn-branch-action" onclick="window.stemApp.switchRepository('${repo.id}')" title="Switch to this repository">
                                                <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                                                Switch
                                            </button>
                                        ` : '<span class="branch-current-label">Active repository</span>'}
                                        <button class="btn-branch-action danger" onclick="window.stemApp.removeRepository('${repo.id}')" title="Remove from list">
                                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M11 1.75V3h2.25a.75.75 0 010 1.5H2.75a.75.75 0 010-1.5H5V1.75C5 .784 5.784 0 6.75 0h2.5C10.216 0 11 .784 11 1.75zM4.496 6.675a.75.75 0 10-1.492.15l.66 6.6A1.75 1.75 0 005.405 15h5.19c.9 0 1.652-.681 1.741-1.576l.66-6.6a.75.75 0 00-1.492-.149l-.66 6.6a.25.25 0 01-.249.225h-5.19a.25.25 0 01-.249-.225l-.66-6.6z"/></svg>
                                            Remove
                                        </button>
                                    </div>
                                </div>
                            `;
                        }).join('')}
                    </div>
                ` : `
                    <div class="empty-state">
                        <svg width="48" height="48" viewBox="0 0 16 16"><path fill="currentColor" d="M2 2.5A2.5 2.5 0 014.5 0h8.75a.75.75 0 01.75.75v12.5a.75.75 0 01-.75.75h-2.5a.75.75 0 110-1.5h1.75v-2h-8a1 1 0 00-.714 1.7.75.75 0 01-1.072 1.05A2.495 2.495 0 012 11.5v-9zm10.5-1V9h-8c-.356 0-.694.074-1 .208V2.5a1 1 0 011-1h8z"/></svg>
                        <h3>No repositories added</h3>
                        <p>Add repositories to quickly switch between them</p>
                        <button class="btn-primary" onclick="window.stemApp.showAddRepoDialog()" style="margin-top: 16px;">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M7.75 2a.75.75 0 01.75.75V7h4.25a.75.75 0 010 1.5H8.5v4.25a.75.75 0 01-1.5 0V8.5H2.75a.75.75 0 010-1.5H7V2.75A.75.75 0 017.75 2z"/></svg>
                            Add Your First Repository
                        </button>
                    </div>
                `}
            </div>
        `;
    }
    
    renderSettings() {
        const config = this.state.config || {
            userName: 'Your Name',
            userEmail: 'your.email@example.com',
            autoStage: false,
            autoSync: false,
            syncInterval: 30
        };
        
        return `
            <div class="view-container">
                <div class="view-header">
                    <h1 class="view-title">Settings</h1>
                    <p class="view-subtitle">Configure STEM VCS</p>
                </div>
                
                <div class="settings-sections">
                    <div class="settings-section">
                        <h3 class="settings-section-title">User Information</h3>
                        <div class="settings-field">
                            <label class="settings-label">Name</label>
                            <input type="text" class="settings-input" id="settingUserName" value="${config.userName}" placeholder="Your Name">
                        </div>
                        <div class="settings-field">
                            <label class="settings-label">Email</label>
                            <input type="email" class="settings-input" id="settingUserEmail" value="${config.userEmail}" placeholder="your.email@example.com">
                        </div>
                    </div>
                    
                    <div class="settings-section">
                        <h3 class="settings-section-title">Repository</h3>
                        <div class="settings-field">
                            <label class="settings-label">Current Branch</label>
                            <div class="settings-value">${this.state.branch}</div>
                        </div>
                        <div class="settings-field">
                            <label class="settings-label">Repository Path</label>
                            <div class="settings-value">${this.repoPath || 'Current directory'}</div>
                        </div>
                    </div>
                    
                    <div class="settings-section">
                        <h3 class="settings-section-title">Automation</h3>
                        <div class="settings-checkbox">
                            <input type="checkbox" id="settingAutoStage" ${config.autoStage ? 'checked' : ''}>
                            <label for="settingAutoStage">Auto-stage modified files</label>
                        </div>
                        <div class="settings-checkbox">
                            <input type="checkbox" id="settingAutoSync" ${config.autoSync ? 'checked' : ''}>
                            <label for="settingAutoSync">Auto-sync with remote</label>
                        </div>
                        <div class="settings-field">
                            <label class="settings-label">Sync Interval (seconds)</label>
                            <input type="number" class="settings-input" id="settingSyncInterval" value="${config.syncInterval}" min="10" max="300">
                        </div>
                    </div>
                    
                    <div class="settings-actions">
                        <button class="btn-primary" onclick="window.stemApp.saveSettings()">
                            <svg width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M13.78 4.22a.75.75 0 010 1.06l-7.25 7.25a.75.75 0 01-1.06 0L2.22 9.28a.75.75 0 011.06-1.06L6 10.94l6.72-6.72a.75.75 0 011.06 0z"/></svg>
                            Save Settings
                        </button>
                        <button class="btn-secondary" onclick="window.stemApp.resetSettings()">
                            Reset to Defaults
                        </button>
                    </div>
                </div>
            </div>
        `;
    }
    
    renderActivityList() {
        const list = document.getElementById('activityList');
        if (!list) return;
        
        const activities = [
            { icon: 'Pushed to main', time: '2 hours ago' },
            { icon: 'Synced', time: '2 hours ago' },
            { icon: 'Committed', time: '2 hours ago' }
        ];
        
        list.innerHTML = activities.map(a => `
            <div class="activity-item">
                <svg class="activity-icon" width="14" height="14" viewBox="0 0 16 16"><path fill="currentColor" d="M8 0a8 8 0 110 16A8 8 0 018 0zM1.5 8a6.5 6.5 0 1113 0 6.5 6.5 0 01-13 0z"/></svg>
                <span class="activity-text">${a.icon}</span>
                <span class="activity-time">${a.time}</span>
            </div>
        `).join('');
    }
    
    renderFolderTree() {
        const tree = document.getElementById('folderTree');
        if (!tree) return;
        
        const folders = [
            { name: '.stem', icon: '📁' },
            { name: 'src', icon: '📁' },
            { name: 'include', icon: '📁' },
            { name: 'README.md', icon: '📄' }
        ];
        
        tree.innerHTML = folders.map(f => `
            <div class="folder-item">
                <span>${f.icon}</span>
                <span style="margin-left: 4px;">${f.name}</span>
            </div>
        `).join('');
    }
    
    attachFileListeners() {
        document.querySelectorAll('.file-item').forEach(item => {
            item.addEventListener('click', () => {
                const idx = parseInt(item.dataset.fileIdx);
                this.selectFile(idx);
            });
        });
    }
    
    attachCommitListeners() {
        document.querySelectorAll('.commit-item').forEach(item => {
            item.addEventListener('click', () => {
                const hash = item.dataset.commit;
                this.selectCommit(hash);
            });
        });
    }
    
    attachSettingsListeners() {
        // Settings are handled with inline onclick for simplicity
        // All inputs update on save button click
    }
    
    attachRepositoryListeners() {
        // Repository actions handled via inline onclick
        // This method exists for future enhancements
    }
    
    selectFile(idx) {
        const file = this.state.files[idx];
        if (!file) return;
        
        this.selectedFile = file;
        
        const preview = document.getElementById('filePreview');
        if (preview) {
            preview.innerHTML = `
                <h3 style="font-size: 13px; font-weight: 600; margin-bottom: 12px; color: var(--text-bright);">${file.path}</h3>
                <div style="font-size: 12px; color: var(--text-secondary); margin-bottom: 16px;">
                    <span style="color: var(--accent-green);">+${file.additions || 0}</span>
                    <span style="color: var(--accent-red); margin-left: 8px;">-${file.deletions || 0}</span>
                </div>
                <div style="background: var(--vscode-bg); padding: 12px; border-radius: 4px; font-family: 'Consolas', monospace; font-size: 12px; color: var(--text-secondary);">
                    Diff preview would appear here
                </div>
            `;
        }
    }
    
    selectCommit(hash) {
        this.selectedCommit = this.state.commits.find(c => c.hash === hash);
    }
    
    async stageAll() {
        this.setLoading(true);
        this.setStatus('Staging all files...');
        try {
            const result = await this.apiCall('/api/stage', {
                method: 'POST',
                body: JSON.stringify({ all: true })
            });
            
            if (result && result.success) {
                await this.loadRepository();
                this.render();
                this.setStatus(result.message || 'All files staged');
            } else {
                this.setStatus('Failed to stage files');
            }
        } catch (error) {
            console.error('Error staging files:', error);
            this.setStatus('Failed to stage files');
        } finally {
            this.setLoading(false);
        }
    }
    
    async unstageAll() {
        this.setLoading(true);
        this.setStatus('Unstaging all files...');
        try {
            // Clear staging file
            const result = await this.apiCall('/api/unstage', {
                method: 'POST',
                body: JSON.stringify({ all: true })
            });
            
            if (result && result.success) {
                await this.loadRepository();
                this.render();
                this.setStatus('All files unstaged');
            } else {
                this.setStatus('Failed to unstage files');
            }
        } catch (error) {
            console.error('Error unstaging files:', error);
            this.setStatus('Failed to unstage files');
        } finally {
            this.setLoading(false);
        }
    }
    
    showCommitDialog() {
        const message = prompt('Enter commit message:');
        if (message) {
            this.commit(message);
        }
    }
    
    async commit(message) {
        this.setLoading(true);
        this.setStatus('Creating commit...');
        try {
            const result = await this.apiCall('/api/commit', {
                method: 'POST',
                body: JSON.stringify({ message })
            });
            
            if (result) {
                await this.loadRepository();
                this.render();
                this.setStatus(`Commit created: ${result.hash}`);
            }
        } catch (error) {
            this.setStatus('Failed to create commit');
        } finally {
            this.setLoading(false);
        }
    }
    
    async pull() {
        this.setLoading(true);
        this.setStatus('Pulling from remote...');
        try {
            const result = await this.apiCall('/api/pull', { method: 'POST' });
            if (result) {
                await this.loadRepository();
                this.render();
                this.setStatus('Pulled successfully');
            }
        } catch (error) {
            this.setStatus('Pull failed');
        } finally {
            this.setLoading(false);
        }
    }
    
    async push() {
        this.setLoading(true);
        this.setStatus('Pushing to remote...');
        try {
            const result = await this.apiCall('/api/push', { method: 'POST' });
            if (result) {
                this.setStatus('Pushed successfully');
            }
        } catch (error) {
            this.setStatus('Push failed');
        } finally {
            this.setLoading(false);
        }
    }
    
    async sync() {
        this.setLoading(true);
        this.setStatus('Syncing...');
        try {
            await this.pull();
            await new Promise(resolve => setTimeout(resolve, 500));
            await this.push();
            this.setStatus('Sync complete');
        } finally {
            this.setLoading(false);
        }
    }
    
    createBranch() {
        const modal = document.getElementById('branchModal');
        if (!modal) {
            // Fallback to prompt if modal doesn't exist yet
            const branchName = prompt('Enter new branch name:');
            if (branchName && branchName.trim()) {
                this.doCreateBranch(branchName.trim());
            }
            return;
        }
        
        const input = document.getElementById('branchNameInput');
        if (input) input.value = '';
        
        modal.classList.add('show');
        setTimeout(() => input?.focus(), 100);
    }
    
    closeBranchModal() {
        const modal = document.getElementById('branchModal');
        if (modal) modal.classList.remove('show');
    }
    
    saveBranch() {
        const input = document.getElementById('branchNameInput');
        const name = input?.value.trim();
        
        if (!name) {
            this.setStatus('Please enter a branch name');
            return;
        }
        
        // Validate branch name (no spaces, special chars)
        if (!/^[a-zA-Z0-9/_-]+$/.test(name)) {
            this.setStatus('Invalid branch name. Use letters, numbers, /, _, or -');
            return;
        }
        
        this.closeBranchModal();
        this.doCreateBranch(name);
    }
    
    async doCreateBranch(branchName) {
        this.setStatus(`Creating branch: ${branchName}...`);
        
        try {
            const result = await this.apiCall('/api/branch/create', {
                method: 'POST',
                body: JSON.stringify({ name: branchName })
            });
            
            if (result && result.success) {
                await this.loadRepository();
                this.render();
                this.setStatus(`Branch '${branchName}' created successfully`);
            } else {
                this.setStatus(`Failed to create branch: ${result?.message || 'Unknown error'}`);
            }
        } catch (error) {
            console.error('Error creating branch:', error);
            this.setStatus(`Failed to create branch: ${error.message}`);
        }
    }
    
    async switchBranch(name) {
        this.setLoading(true);
        this.setStatus(`Switching to ${name}...`);
        
        // Update current branch in .stem/HEAD
        try {
            const result = await this.apiCall('/api/branch/switch', {
                method: 'POST',
                body: JSON.stringify({ branch: name })
            });
            
            if (result && result.success) {
                // Update local state immediately
                this.state.branch = name;
                this.state.branches = this.state.branches.map(b => ({
                    ...b,
                    current: b.name === name
                }));
                
                await this.loadRepository();
                this.render();
                this.setStatus(`Switched to ${name}`);
            } else {
                this.setStatus(`Failed to switch branch: ${result?.message || 'Unknown error'}`);
            }
        } catch (error) {
            console.error('Error switching branch:', error);
            this.setStatus(`Failed to switch branch: ${error.message}`);
        } finally {
            this.setLoading(false);
        }
    }
    
    merge() {
        // Show merge dialog with branch selection
        const branches = this.state.branches.filter(b => !b.current);
        
        if (branches.length === 0) {
            this.setStatus('No other branches to merge');
            return;
        }
        
        // Create a simple modal to select branch
        const branchOptions = branches.map(b => `<option value="${b.name}">${b.name}</option>`).join('');
        
        this.showConfirmDialog(
            'Merge Branch',
            `
                <div style="margin-bottom: 12px;">
                    <label style="display: block; margin-bottom: 8px; color: var(--text-secondary);">Select branch to merge into <strong>${this.state.branch}</strong>:</label>
                    <select id="mergeBranchSelect" style="width: 100%; padding: 8px; background: var(--vscode-input-bg); border: 1px solid var(--vscode-border); color: var(--text-primary); border-radius: 4px;">
                        ${branchOptions}
                    </select>
                </div>
            `,
            'This will merge the selected branch into the current branch',
            () => {
                const select = document.getElementById('mergeBranchSelect');
                const sourceBranch = select ? select.value : branches[0].name;
                this.mergeBranch(sourceBranch);
            }
        );
    }
    
    async deleteBranch(name) {
        // Show confirmation modal
        this.showConfirmDialog(
            'Delete Branch',
            `Delete branch "${name}"?`,
            'This action cannot be undone',
            async () => {
                this.setStatus(`Deleting branch: ${name}...`);
                
                try {
                    const result = await this.apiCall('/api/branch/delete', {
                        method: 'POST',
                        body: JSON.stringify({ branch: name })
                    });
                    
                    if (result && result.success) {
                        await this.loadRepository();
                        this.render();
                        this.setStatus(`Branch '${name}' deleted`);
                    } else {
                        this.setStatus(`Failed to delete branch: ${result?.message || 'Unknown error'}`);
                    }
                } catch (error) {
                    console.error('Error deleting branch:', error);
                    this.setStatus(`Failed to delete branch: ${error.message}`);
                }
            }
        );
    }
    
    async mergeBranch(sourceBranch) {
        // Show confirmation modal
        this.showConfirmDialog(
            'Merge Branch',
            `Merge "${sourceBranch}" into "${this.state.branch}"?`,
            null,
            async () => {
                this.setLoading(true);
                this.setStatus(`Merging ${sourceBranch} into ${this.state.branch}...`);
                
                try {
                    const result = await this.apiCall('/api/branch/merge', {
                        method: 'POST',
                        body: JSON.stringify({ 
                            source: sourceBranch,
                            target: this.state.branch
                        })
                    });
                    
                    if (result && result.success) {
                        await this.loadRepository();
                        this.render();
                        this.setStatus(`Merged ${sourceBranch} successfully`);
                    } else {
                        this.setStatus(`Merge failed: ${result?.message || 'Unknown error'}`);
                    }
                } catch (error) {
                    console.error('Error merging branch:', error);
                    this.setStatus(`Merge failed: ${error.message}`);
                } finally {
                    this.setLoading(false);
                }
            }
        );
    }
    
    createTag() {
        const modal = document.getElementById('tagModal');
        const input = document.getElementById('tagNameInput');
        
        if (input) input.value = '';
        modal.classList.add('show');
        setTimeout(() => input?.focus(), 100);
    }
    
    closeTagModal() {
        const modal = document.getElementById('tagModal');
        if (modal) modal.classList.remove('show');
    }
    
    saveTag() {
        const input = document.getElementById('tagNameInput');
        const name = input?.value.trim();
        
        if (!name) {
            this.setStatus('Please enter a tag name');
            return;
        }
        
        // Validate tag name
        if (!/^[a-zA-Z0-9._-]+$/.test(name)) {
            this.setStatus('Invalid tag name. Use letters, numbers, dots, underscores, or hyphens');
            return;
        }
        
        this.closeTagModal();
        this.doCreateTag(name);
    }
    
    async doCreateTag(tagName) {
        this.setStatus(`Creating tag: ${tagName}...`);
        
        try {
            const result = await this.apiCall('/api/tag/create', {
                method: 'POST',
                body: JSON.stringify({ name: tagName })
            });
            
            if (result && result.success) {
                await this.loadRepository();
                this.render();
                this.setStatus(`Tag '${tagName}' created`);
            } else {
                this.setStatus(`Failed to create tag: ${result?.message || 'Unknown error'}`);
            }
        } catch (error) {
            console.error('Error creating tag:', error);
            this.setStatus(`Failed to create tag: ${error.message}`);
        }
    }
    
    async deleteTag(name) {
        // Show confirmation modal
        this.showConfirmDialog(
            'Delete Tag',
            `Delete tag "${name}"?`,
            'This action cannot be undone',
            async () => {
                this.setStatus(`Deleting tag: ${name}...`);
                
                try {
                    const result = await this.apiCall('/api/tag/delete', {
                        method: 'POST',
                        body: JSON.stringify({ tag: name })
                    });
                    
                    if (result && result.success) {
                        await this.loadRepository();
                        this.render();
                        this.setStatus(`Tag '${name}' deleted`);
                    } else {
                        this.setStatus(`Failed to delete tag: ${result?.message || 'Unknown error'}`);
                    }
                } catch (error) {
                    console.error('Error deleting tag:', error);
                    this.setStatus(`Failed to delete tag: ${error.message}`);
                }
            }
        );
    }
    
    resetSettings() {
        // Show confirmation modal
        this.showConfirmDialog(
            'Reset Settings',
            'Reset all settings to defaults?',
            null,
            () => {
                this.state.config = {
                    userName: 'Your Name',
                    userEmail: 'your.email@example.com',
                    autoStage: false,
                    autoSync: false,
                    syncInterval: 30
                };
                
                this.render();
                this.setStatus('Settings reset to defaults');
            }
        );
    }
    
    saveSettings() {
        const config = {
            userName: document.getElementById('settingUserName')?.value || '',
            userEmail: document.getElementById('settingUserEmail')?.value || '',
            autoStage: document.getElementById('settingAutoStage')?.checked || false,
            autoSync: document.getElementById('settingAutoSync')?.checked || false,
            syncInterval: parseInt(document.getElementById('settingSyncInterval')?.value || '30')
        };
        
        // Save to backend
        this.apiCall('/api/config', {
            method: 'POST',
            body: JSON.stringify(config)
        }).then(result => {
            if (result && result.success) {
                this.state.config = config;
                this.setStatus('Settings saved');
            } else {
                this.setStatus('Failed to save settings');
            }
        });
    }
    
    // Generic confirmation dialog
    showConfirmDialog(title, message, warning, onConfirm) {
        this.pendingConfirmAction = onConfirm;
        
        const modal = document.getElementById('confirmModal');
        const titleEl = document.getElementById('confirmTitle');
        const messageEl = document.getElementById('confirmMessage');
        const warningEl = document.getElementById('confirmWarning');
        
        if (titleEl) titleEl.textContent = title;
        if (messageEl) {
            // Support HTML content
            messageEl.innerHTML = message;
        }
        
        if (warning && warningEl) {
            warningEl.textContent = `⚠️ ${warning}`;
            warningEl.style.display = 'block';
        } else if (warningEl) {
            warningEl.style.display = 'none';
        }
        
        modal.classList.add('show');
    }
    
    closeConfirmModal() {
        const modal = document.getElementById('confirmModal');
        modal.classList.remove('show');
        this.pendingConfirmAction = null;
    }
    
    confirmAction() {
        if (this.pendingConfirmAction) {
            this.pendingConfirmAction();
        }
        this.closeConfirmModal();
    }
    
    // Repository Management
    updateRepositorySelector() {
        const select = document.getElementById('repoSelect');
        if (!select) return;
        
        select.innerHTML = '<option value="current">Current Repository</option>';
        
        this.repositories.forEach(repo => {
            const option = document.createElement('option');
            option.value = repo.id;
            option.textContent = repo.name;
            if (repo.id === this.currentRepoId) {
                option.selected = true;
            }
            select.appendChild(option);
        });
    }
    
    showAddRepoDialog() {
        const modal = document.getElementById('addRepoModal');
        const nameInput = document.getElementById('repoNameInput');
        const pathInput = document.getElementById('repoPathInput');
        const remoteInput = document.getElementById('repoRemoteInput');
        
        if (nameInput) nameInput.value = '';
        if (pathInput) pathInput.value = '';
        if (remoteInput) remoteInput.value = '';
        
        modal.classList.add('show');
        setTimeout(() => nameInput?.focus(), 100);
    }
    
    closeAddRepoModal() {
        const modal = document.getElementById('addRepoModal');
        if (modal) modal.classList.remove('show');
    }
    
    async saveRepository() {
        const name = document.getElementById('repoNameInput')?.value.trim();
        const path = document.getElementById('repoPathInput')?.value.trim();
        const remote = document.getElementById('repoRemoteInput')?.value.trim();
        
        if (!name) {
            this.setStatus('Please enter a repository name');
            return;
        }
        
        if (!path) {
            this.setStatus('Please enter a repository path');
            return;
        }
        
        // Create repository object
        const repo = {
            id: Date.now().toString(),
            name: name,
            path: path,
            remote: remote || null,
            addedAt: new Date().toISOString()
        };
        
        // Add to repositories list
        this.repositories.push(repo);
        this.saveRepositoriesList();
        
        this.closeAddRepoModal();
        this.updateRepositorySelector();
        this.setStatus(`Repository '${name}' added`);
    }
    
    async switchRepository(repoId) {
        console.log('[switchRepository] Called with repoId:', repoId);
        
        if (repoId === 'current') {
            console.log('[switchRepository] Ignoring switch to current');
            return;
        }
        
        const repo = this.repositories.find(r => r.id === repoId);
        if (!repo) {
            console.log('[switchRepository] Repository not found:', repoId);
            return;
        }
        
        console.log('[switchRepository] Found repo:', repo);
        
        this.setCurrentRepo(repoId);
        this.repoPath = repo.path;
        
        this.setLoading(true);
        this.setStatus(`Switching to ${repo.name}...`);
        
        // Call backend to switch directory
        try {
            console.log('[switchRepository] Calling API with path:', repo.path);
            const result = await this.apiCall('/api/repo/switch', {
                method: 'POST',
                body: JSON.stringify({ path: repo.path })
            });
            
            console.log('[switchRepository] API result:', result);
            
            if (result && result.success) {
                // Load repository data (this will read the remote from .stem/remote.txt)
                console.log('[switchRepository] Loading repository data...');
                await this.loadRepository();
                
                // Update the repo's stored remote from what was actually loaded
                if (this.state.remote && this.state.remote.url) {
                    repo.remote = this.state.remote.url;
                } else {
                    repo.remote = null;
                }
                this.saveRepositoriesList();
                
                console.log('[switchRepository] Rendering...');
                this.render();
                this.setStatus(`Switched to repository: ${repo.name}`);
            } else {
                console.error('[switchRepository] API failed:', result);
                this.setStatus('Failed to switch repository: ' + (result?.message || 'Unknown error'));
            }
        } catch (error) {
            console.error('[switchRepository] Error:', error);
            this.setStatus('Failed to switch repository: ' + error.message);
        } finally {
            this.setLoading(false);
        }
    }
    
    removeRepository(repoId) {
        const repo = this.repositories.find(r => r.id === repoId);
        if (!repo) return;
        
        this.showConfirmDialog(
            'Remove Repository',
            `Remove "${repo.name}" from the list?`,
            'This will not delete any files',
            () => {
                this.repositories = this.repositories.filter(r => r.id !== repoId);
                this.saveRepositoriesList();
                
                if (this.currentRepoId === repoId) {
                    this.setCurrentRepo('current');
                }
                
                this.updateRepositorySelector();
                this.setStatus(`Repository '${repo.name}' removed from list`);
            }
        );
    }
    
    changeRemote() {
        const modal = document.getElementById('remoteModal');
        const input = document.getElementById('remoteUrlInput');
        
        // Set current URL as default
        input.value = this.state.remote?.url || '';
        
        // Show modal
        modal.classList.add('show');
        
        // Focus input
        setTimeout(() => input.focus(), 100);
    }
    
    closeRemoteModal() {
        const modal = document.getElementById('remoteModal');
        modal.classList.remove('show');
    }
    
    saveRemoteUrl() {
        const input = document.getElementById('remoteUrlInput');
        const url = input.value.trim();
        
        if (!url) {
            this.setStatus('Please enter a valid URL');
            return;
        }
        
        this.closeRemoteModal();
        this.setRemote(url);
    }
    
    showCommitDialog() {
        const modal = document.getElementById('commitModal');
        const textarea = document.getElementById('commitMessageInput');
        const fileCount = document.getElementById('stagedFilesCount');
        
        // Update staged files count
        const staged = this.state.staged.length;
        fileCount.textContent = `${staged} file${staged !== 1 ? 's' : ''} staged for commit`;
        
        // Clear previous message
        textarea.value = '';
        
        // Show modal
        modal.classList.add('show');
        
        // Focus textarea
        setTimeout(() => textarea.focus(), 100);
    }
    
    closeCommitModal() {
        const modal = document.getElementById('commitModal');
        modal.classList.remove('show');
    }
    
    saveCommit() {
        const textarea = document.getElementById('commitMessageInput');
        const message = textarea.value.trim();
        
        if (!message) {
            this.setStatus('Please enter a commit message');
            return;
        }
        
        this.closeCommitModal();
        this.commit(message);
    }
    
    async setRemote(url) {
        this.setLoading(true);
        this.setStatus('Updating remote...');
        try {
            const result = await this.apiCall('/api/remote', {
                method: 'POST',
                body: JSON.stringify({ url })
            });
            
            if (result && result.success) {
                // Update state immediately
                if (!this.state.remote) this.state.remote = {};
                this.state.remote.url = url;
                this.state.remote.connected = true;
                this.state.remote.name = 'Google Drive';
                
                // Update the current repository's cached remote in localStorage
                if (this.currentRepoId !== 'current') {
                    const repo = this.repositories.find(r => r.id === this.currentRepoId);
                    if (repo) {
                        repo.remote = url;
                        this.saveRepositoriesList();
                    }
                }
                
                // Force immediate UI update
                this.updateRightPanel();
                
                this.setStatus('Remote updated: ' + url.substring(0, 50) + '...');
                
                // Also reload from server to confirm
                setTimeout(() => this.loadRepository(), 500);
            } else {
                this.setStatus('Failed to update remote: ' + (result?.message || 'Unknown error'));
            }
        } catch (error) {
            console.error('Error updating remote:', error);
            this.setStatus('Failed to update remote: ' + error.message);
        } finally {
            this.setLoading(false);
        }
    }
    
    search(query) {
        this.setStatus(`Searching for: ${query}`);
    }
    
    setStatus(message) {
        const statusEl = document.getElementById('statusMessage');
        if (statusEl) {
            statusEl.textContent = message;
        }
        
        setTimeout(() => {
            if (statusEl && statusEl.textContent === message) {
                statusEl.textContent = 'Ready';
            }
        }, 3000);
    }
    
    setLoading(isLoading) {
        this.loading = isLoading;
        const body = document.body;
        
        if (isLoading) {
            body.classList.add('loading');
            // Show loading indicator in status bar
            this.setStatus('Processing...');
        } else {
            body.classList.remove('loading');
        }
    }
    
    startAutoRefresh() {
        setInterval(() => {
            this.loadRepository();
        }, 30000); // Refresh every 30 seconds
    }
}

// Initialize app when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        window.stemApp = new STEMApp();
    });
} else {
    window.stemApp = new STEMApp();
}
