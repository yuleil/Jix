#include "types.h"
#include "def.h"
#include "fs.h"
#include "global.h"
#include "cache.h"

static void itrunc(struct inode*);

static void
readsb(int dev, struct superblock *sb)
{
    struct buf *bp;

    bp = bread(dev, ISTART - 1);
    memcpy(sb, bp->data, sizeof(*sb));
    brelse(bp);
}



static void
bzero(int dev, int bno)
{
    struct buf *bp;

    bp = bread(dev, bno);
    memset(bp->data, 0, BSIZE);
    bwrite(bp);
    brelse(bp);
}

// Blocks.

// Allocate a zeroed disk block.
static u32
balloc(u32 dev)
{
    u32 b, bi;
    u8 mask;
    struct buf *bp = NULL;
    struct superblock sb;

    readsb(dev, &sb);

    for (b = 0; b < sb.size; b += BPB) {
        bp = bread(dev, BBLOCK(b, sb.bitmapstart));
        for (bi = 0; bi < BPB && bi < (sb.size - b); bi++) {
            mask = 1 << (bi % 8);
            if ((bp->data[bi/8] & mask) == 0) {  // Is block free?
                bp->data[bi/8] |= mask;  // Mark block in use on disk.
                bwrite(bp);
                brelse(bp);
                bzero(dev, b + bi);
                return b + bi;
            }
        }
        brelse(bp);
    }
    panic("balloc: out of blocks");
    return -1;
}

// Free a disk block.
static void
bfree(int dev, u32 b)
{
    struct buf *bp;
    struct superblock sb;
    int bi, mask;

    readsb(dev, &sb);
    bp = bread(dev, BBLOCK(b, sb.bitmapstart));
    bi = b % BPB;
    mask = 1 << (bi % 8);
    if ((bp->data[bi/8] & mask) == 0)
        panic("freeing free block");
    bp->data[bi/8] &= ~mask;  // Mark block free on disk.
    bwrite(bp);
    brelse(bp);
}



// Find the inode with number inum on device dev
// and return the in-memory copy.
static struct inode *
iget(u32 dev, u32 inum)
{
    struct inode *ip, *empty;
    struct dinode *dip;
    struct buf *bp;

    // Try for cached inode.
    empty = NULL;
    for (ip = inodes; ip < inodes + NINODE; ip++) {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            return ip;
        }
        if (empty == NULL && ip->ref == 0)    // Remember empty slot.
            empty = ip;
    }

    // Allocate fresh inode.
    if (empty == NULL)
        panic("iget: inodes overflow");

    ip = empty;

    ip->dev     = dev;
    ip->inum    = inum;
    ip->ref     = 1;
    bp          = bread(ip->dev, IBLOCK(ip->inum));
    dip         = (struct dinode *)bp->data + ip->inum % IPB;
    ip->type    = dip->type;
    ip->major   = dip->major;
    ip->minor   = dip->minor;
    ip->nlink   = dip->nlink;
    ip->size    = dip->size;
    memcpy(ip->addrs, dip->addrs, sizeof(ip->addrs));
    brelse(bp);

    return ip;
}


// Allocate a new inode with the given type on device dev.
struct inode *
ialloc(u32 dev, short type)
{
    int inum;
    struct buf *bp;
    struct dinode *dip;
    struct superblock sb;

    readsb(dev, &sb);
    for (inum = 1; inum < sb.ninodes; inum++) {  // loop over inode blocks
        bp = bread(dev, IBLOCK(inum));
        dip = (struct dinode *)bp->data + inum % IPB;

        if (dip->type == 0) {  // free inode
            memset(dip, 0, sizeof(*dip));
            dip->type = type; // mark using
            bwrite(bp);
            brelse(bp);
            return iget(dev, inum);
        }
        brelse(bp);
    }
    panic("ialloc: no inodes");
    return NULL;
}

// write inode back to disk
void
iupdate(struct inode *ip)
{
    struct buf *bp;
    struct dinode *dip;

    bp = bread(ip->dev, IBLOCK(ip->inum));
    dip = (struct dinode*)bp->data + ip->inum % IPB;
    dip->type = ip->type;
    dip->nlink = ip->nlink;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->size = ip->size;
    memcpy(dip->addrs, ip->addrs, sizeof(ip->addrs));
    bwrite(bp);
    brelse(bp);
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode *
idup(struct inode *ip)
{
    assert(ip->ref > 0);
    ip->ref++;
    return ip;
}


// Drop reference.
void
iput(struct inode *ip)
{
    assert(ip->ref > 0);
    if (ip->ref == 1 && ip->nlink == 0) {
        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
    }
    ip->ref--;
}


// return ip's bnth data block
static u32
bmap(struct inode *ip, u32 bn)
{
    u32 addr, *a;
    struct buf *bp;

    if (bn < NDIRECT) {
        if((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = balloc(ip->dev);
        return addr;
    }
    bn -= NDIRECT;
    if (bn < NINDIRECT) {
        // Load indirect block, allocating if necessary.
        if ((addr = ip->addrs[NDIRECT]) == 0)
            ip->addrs[NDIRECT] = addr = balloc(ip->dev);
        bp = bread(ip->dev, addr);
        a = (u32 *)bp->data;
        if ((addr = a[bn]) == 0) {
            a[bn] = addr = balloc(ip->dev);
            bwrite(bp);
        }
        brelse(bp);
        return addr;
    }

    panic("bmap: out of range");
    return -1;  // eliminate  warming.
}

// Truncate inode (discard contents).
// Only called after the last dirent referring
// to this inode has been erased on disk.
static void
itrunc(struct inode *ip)
{
    struct buf *bp;
    u32 i, *a;

    for (i = 0; i < NDIRECT; i++) {
        if (ip->addrs[i]) {
            bfree(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    if(ip->addrs[NDIRECT]){
        bp = bread(ip->dev, ip->addrs[NDIRECT]);
        a = (u32 *)bp->data;
        for(i = 0; i < NINDIRECT; i++){
            if(a[i])
                bfree(ip->dev, a[i]);
        }
        brelse(bp);
        bfree(ip->dev, ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }

    ip->size = 0;
    iupdate(ip);
}

// Copy stat information from inode.
void
stati(struct inode *ip, struct stat *st)
{
    st->dev = ip->dev;
    st->ino = ip->inum;
    st->type = ip->type;
    st->nlink = ip->nlink;
    st->size = ip->size;
}

// Read data from inode.
int
readi(struct inode *ip, char *dst, u32 off, u32 n)
{
    u32 tot, m;
    struct buf *bp;

    if (ip->type == T_DEV)
        return tty_read(dst, n);

    if (off > ip->size)
        return -1;
    if (off + n > ip->size)
        n = ip->size - off;

    for (tot = 0; tot < n; tot += m, off += m, dst += m) {
        bp = bread(ip->dev, bmap(ip, off / BSIZE));
        m = min(n - tot, BSIZE - off % BSIZE);
        memcpy(dst, bp->data + off % BSIZE, m);
        brelse(bp);
    }
    return n;
}

// Write data to inode.
int
writei(struct inode *ip, char *src, u32 off, u32 n)
{
    u32 tot, m;
    struct buf *bp;

    if (ip->type == T_DEV)
        return tty_write(src, n);

    if (off > ip->size || off + n > MAXFILE * BSIZE)
        return -1;

    for (tot = 0; tot < n; tot += m, off += m, src += m) {
        bp = bread(ip->dev, bmap(ip, off / BSIZE));
        m = min(n - tot, BSIZE - off % BSIZE);
        memcpy(bp->data + off % BSIZE, src, m);
        bwrite(bp);
        brelse(bp);
    }

    if(n > 0 && off > ip->size){
        ip->size = off;
        iupdate(ip);
    }
    return n;
}


// Directories

int
namecmp(const char *s, const char *t)
{
    return strncmp(s, t, NAMELEN);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
struct inode *
dirlookup(struct inode *dp, char *name, u32 *poff)
{
    u32 off;
    struct dirent de;
    //assert(dp);

    if (dp->type != T_DIR)
        panic("dirlookup not DIR");

    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
            panic("dirlookup read");
        if (de.inum == 0)
            continue;
        if (namecmp(name, de.name) == 0) {

            // entry matches path element
            if(poff)
                *poff = off;
            return iget(dp->dev, de.inum);
        }
    }

    return NULL;
}

// Write a new directory entry (name, inum) into the directory dp.
int
dirlink(struct inode *dp, char *name, u32 inum)
{
    int off;
    struct dirent de;
    struct inode *ip;

    // Check that name is not present.
    if ((ip = dirlookup(dp, name, 0)) != NULL) {
        iput(ip);
        return -1;
    }

    // Look for an empty dirent.
    for (off = 0; off < dp->size; off += sizeof(de)) {
        if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            panic("dirlink read");
        if(de.inum == 0)   //found
            break;
    }

    strncpy(de.name, name, NAMELEN);
    de.inum = inum;

    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("dirlink");

    return 0;
}


static const char *  // return remain path
skipelem(const char *path, char *name)
{
    const char *s;
    int len;

    while (*path == '/')
        path++;

    if (*path == '\0')
        return NULL;

    s = path;
    while (*path != '/' && *path != '\0')
        path++;
    len = path - s;

    if(len >= NAMELEN)
        memcpy(name, s, NAMELEN);
    else {
        memcpy(name, s, len);
        name[len] = '\0';
    }
    while(*path == '/')
        path++;
    return path;
}

// Look up and return the inode for a path name.
// If nameiparent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for NAMELEN bytes.
static struct inode *
namex(const char *path, int parent, char *name)
{
    struct inode *ip, *next;

    if (!fs_current->cwd)
        fs_current->cwd = iget(ROOTDEV, ROOTINO);

    if (*path == '/')
        ip = iget(ROOTDEV, ROOTINO);
    else
        ip = idup(fs_current->cwd);

    while ((path = skipelem(path, name)) != NULL) {
        if (ip->type != T_DIR)
            return NULL;

        if (parent && *path == '\0')
            return ip;

        if ((next = dirlookup(ip, name, NULL)) == NULL) {
            iput(ip);
            return NULL;
        }
        iput(ip); // dont need anymore
        ip = next;
    }
    if (parent) {  //should return early
        iput(ip);
        return NULL;
    }
    return ip;
}

struct inode *
namei(const char *path)
{
    char name[NAMELEN];
    return namex(path, 0, name);
}

struct inode *
nameiparent(const char *path, char *name)
{
    return namex(path, 1, name);
}




struct inode *
icreate(const char *path, short type, short major, short minor)
{
    struct inode *ip, *dp;
    char name[NAMELEN];

    if ((dp = nameiparent(path, name)) == NULL)
        return NULL;

    if ((ip = dirlookup(dp, name, NULL)) != NULL) { // present
        iput(dp);
        if (type == T_FILE && ip->type == T_FILE)
            return ip;
        iput(ip);
        return NULL;
    }


    if ((ip = ialloc(dp->dev, type)) == NULL)
            panic("icreate: ialloc");
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);

    if (ip->type == T_DIR) {  // create . and .. entries.
        dp->nlink++;  // for "ip/.."
        iupdate(dp);
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
            panic("icreate: link dots");
    }

    if(dirlink(dp, name, ip->inum) < 0)
        panic("icreate: dirlink");

    iput(dp);
    return ip;
}
