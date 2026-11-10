# STEM Architecture

This document describes the internal architecture of STEM VCS.

## Overview

STEM is built as a modular system with clear separation between core VCS operations, storage backends, and user interfaces.

```
┌─────────────────────────────────────────────────────┐
│                  User Interfaces                    │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────┐│
│  │  CLI Tool    │  │  GUI (LVGL)  │  │ IDE Plugin││
│  └──────┬───────┘  └──────┬───────┘  └─────┬─────┘│
└─────────┼──────────────────┼────────────────┼──────┘
          │                  │                │
┌─────────▼──────────────────▼────────────────▼──────┐
│              STEM Core Library (C99)                │
│  ┌─────────────────────────────────────────────┐   │
│  │  Repository Management                      │   │
│  │  - Init, Open, Close                        │   │
│  │  - Config Management                        │   │
│  │  - Reference Management (HEAD, branches)    │   │
│  └─────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────┐   │
│  │  Object Store                               │   │
│  │  - Content-Addressable Storage              │   │
│  │  - Blob, Tree, Commit Objects               │   │
│  │  - SHA-256 Hashing                          │   │
│  │  - Object Compression                       │   │
│  └─────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────┐   │
│  │  Index (Staging Area)                       │   │
│  │  - File tracking                            │   │
│  │  - Change detection                         │   │
│  │  - Partial staging                          │   │
│  └─────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────┐   │
│  │  Merge Engine                               │   │
│  │  - Three-way merge                          │   │
│  │  - Conflict detection                       │   │
│  │  - Strategy selection                       │   │
│  └─────────────────────────────────────────────┘   │
└─────────────────────┬───────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────┐
│           Storage Provider Interface (SPI)          │
│  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐ │
│  │Connect│  │Upload│  │Download│ │Delete│  │List  │ │
│  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘ │
└─────────────────────┬───────────────────────────────┘
                      │
    ┌─────────────────┼─────────────────┐
    │                 │                 │
┌───▼────┐  ┌────────▼────────┐  ┌─────▼─────┐
│ Cloud  │  │  S3/R2/Azure    │  │Local/NAS  │
│Providers│  │  Object Storage │  │ Filesystem│
│GDrive │  │  Compatible     │  │  Providers│
│Dropbox │  │                 │  │           │
│OneDrive│  │                 │  │           │
└────────┘  └─────────────────┘  └───────────┘
```

## Core Components

### 1. Repository (`src/core/repository.c`)

**Responsibilities:**
- Initialize new repositories
- Open existing repositories
- Discover repositories in parent directories
- Manage repository metadata

**Key Functions:**
- `stem_repository_init()` - Create new repository
- `stem_repository_open()` - Open existing repository
- `stem_repository_discover()` - Find .stem directory
- `stem_repository_free()` - Clean up resources

**Data Structures:**
```c
struct stem_repository {
    char *path;              // Working directory
    char *stem_dir;          // .stem/ directory
    stem_config_t *config;   // Configuration
    stem_index_t *index;     // Staging area
    bool is_bare;            // Bare repository flag
};
```

### 2. Object Store (`src/core/object.c`)

**Responsibilities:**
- Store immutable objects
- Content-addressable access
- Object serialization/deserialization
- Deduplication

**Object Types:**
- **Blob**: Raw file content
- **Tree**: Directory structure
- **Commit**: Snapshot metadata

**Key Functions:**
- `stem_blob_create()` - Create blob from data
- `stem_tree_create()` - Create empty tree
- `stem_tree_add_entry()` - Add entry to tree
- `stem_commit_create()` - Create commit object

**Storage Format:**
```
.stem/objects/
├── ab/
│   └── 123456...  # Object file (first 2 chars = subdirectory)
├── cd/
│   └── 789abc...
...
```

### 3. Index (`src/core/index.c`)

**Responsibilities:**
- Track working directory changes
- Stage files for commit
- Maintain file metadata

**Key Functions:**
- `stem_index_open()` - Load index
- `stem_index_add()` - Stage file
- `stem_index_remove()` - Unstage file
- `stem_index_write()` - Save index

**Index Entry:**
```c
typedef struct {
    char *path;           // File path
    stem_hash_t hash;     // Content hash
    uint32_t mode;        // File permissions
    uint64_t size;        // File size
    stem_time_t mtime;    // Modification time
    uint32_t flags;       // Status flags
} stem_index_entry_t;
```

### 4. References (`src/core/reference.c`)

**Responsibilities:**
- Manage branches and tags
- Handle HEAD pointer
- Resolve symbolic references

**Reference Types:**
- **Direct**: Points to commit hash
- **Symbolic**: Points to another reference (e.g., HEAD → refs/heads/main)

**Key Functions:**
- `stem_reference_lookup()` - Find reference
- `stem_reference_create()` - Create new reference
- `stem_branch_create()` - Create branch
- `stem_branch_checkout()` - Switch branches

### 5. Configuration (`src/core/config.c`)

**Responsibilities:**
- Store repository settings
- User preferences
- Remote storage configuration

**Key Functions:**
- `stem_config_open()` - Load config
- `stem_config_get_string()` - Read setting
- `stem_config_set_string()` - Write setting
- `stem_config_save()` - Persist changes

**Config Format (JSON):**
```json
{
  "core": {
    "repositoryformatversion": 1,
    "bare": false
  },
  "user": {
    "name": "John Doe",
    "email": "john@example.com"
  },
  "storage": {
    "provider": "gdrive",
    "url": "gdrive://MyDrive/Projects/MyRepo",
    "encryption": true
  }
}
```

## Storage Provider Interface

### Provider Structure

```c
typedef struct stem_storage_provider {
    stem_storage_type_t type;
    char *name;
    
    /* Operations */
    stem_error_t (*connect)(struct stem_storage_provider *);
    stem_error_t (*upload)(struct stem_storage_provider *, 
                           const char *key, 
                           const uint8_t *data, 
                           size_t size);
    stem_error_t (*download)(struct stem_storage_provider *, 
                             const char *key, 
                             uint8_t **data_out, 
                             size_t *size_out);
    stem_error_t (*delete)(struct stem_storage_provider *, 
                           const char *key);
    stem_error_t (*list)(struct stem_storage_provider *, 
                         const char *prefix, 
                         char ***keys_out, 
                         size_t *count_out);
    
    void *_priv;  // Provider-specific data
} stem_storage_provider_t;
```

### Implementing a Provider

1. Create provider struct
2. Implement connection handling
3. Implement CRUD operations
4. Handle authentication
5. Register provider

Example:
```c
stem_error_t my_provider_upload(stem_storage_provider_t *provider,
                                 const char *key,
                                 const uint8_t *data,
                                 size_t size) {
    /* 1. Get provider private data */
    my_provider_data_t *priv = provider->_priv;
    
    /* 2. Build HTTP request */
    /* 3. Add authentication */
    /* 4. Upload data */
    /* 5. Verify response */
    
    return STEM_OK;
}
```

## Synchronization Strategy

### Object Upload Flow

```
1. User commits changes
2. New objects created locally
3. Background sync daemon detects new objects
4. Objects queued for upload
5. Parallel upload to cloud storage
6. Mark objects as synced
7. Update sync log
```

### Object Download Flow (Lazy)

```
1. User checks out different commit
2. Check local object cache
3. If not found, check cloud storage
4. Download object
5. Cache locally
6. Reconstruct working directory
```

### Conflict Resolution

```
1. Detect concurrent modifications
2. Attempt automatic merge
3. If conflicts, create conflict markers
4. Notify user
5. User resolves conflicts
6. Create merge commit
```

## Data Flow Example: Commit

```
User: stem commit -m "Add feature"
         │
         ▼
    ┌─────────────────┐
    │  Parse Command  │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │   Read Index    │ (Staged files)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Create Blobs   │ (For each file)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Create Tree    │ (Directory structure)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │ Create Commit   │ (With message, author, tree)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │ Update HEAD     │ (Point to new commit)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  Clear Index    │ (Reset staging)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │Queue Upload Job │ (Background sync)
    └─────────────────┘
```

## Performance Considerations

### Object Lookups: O(1)

Objects accessed by hash via filesystem:
```
.stem/objects/ab/cd1234...
                ^^
        Directory (256 buckets)
```

### Binary Search for Index

Index entries sorted by path for fast lookups.

### Lazy Loading

- Don't load entire repository into memory
- Stream large files
- Cache frequently accessed objects

### Parallel Operations

- Upload multiple objects simultaneously
- Download in parallel during clone
- Background sync while user works

## Security Model

### Hash Verification

Every object verified via SHA-256:
```
Advertised Hash → Computed Hash
        │               │
        └───── Match? ──┘
                │
        Yes: Accept │ No: Reject
```

### Encryption (Planned)

```
Plaintext → Encrypt → Upload → Cloud Storage
                               (Encrypted)
Download → Decrypt → Plaintext
```

### Access Control

- OAuth tokens for cloud providers
- Encrypted credentials in config
- No passwords in plain text

## Extension Points

### Custom Storage Providers

Implement `stem_storage_provider_t` interface.

### Custom Merge Strategies

Register merge strategy callback.

### Hooks (Future)

- Pre-commit
- Post-commit
- Pre-push
- Post-merge

## Testing Strategy

### Unit Tests

Test individual functions in isolation.

### Integration Tests

Test component interactions.

### End-to-End Tests

Test complete workflows from CLI/GUI.

### Fuzzing

Randomized testing for robustness.

## Future Enhancements

- **Parallel Merkle Tree**: Faster verification
- **Bloom Filters**: Quick object existence checks
- **Delta Compression**: Smaller storage
- **Partial Clones**: Download only needed objects
- **Smart Protocol**: Efficient fetch/push

---

This architecture balances simplicity, performance, and extensibility while staying true to the pure C implementation philosophy.
