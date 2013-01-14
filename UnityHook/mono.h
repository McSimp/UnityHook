// Retarded definitions
typedef unsigned char guint8;
typedef unsigned short guint16;
typedef unsigned int guint32;
typedef int gint;
typedef gint gboolean;

typedef unsigned char uint8_t;
typedef uint8_t mono_byte;
typedef unsigned int uint16_t;
typedef unsigned long int uint32_t;

typedef void* gpointer;

struct GSList {
  gpointer data;
  GSList *next;
};

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoAssemblyName MonoAssemblyName;
typedef struct _MonoImage MonoImage;

#define MONO_PUBLIC_KEY_TOKEN_LENGTH	17

struct _MonoAssemblyName {
	const char *name;
	const char *culture;
	const char *hash_value;
	const mono_byte* public_key;
	// string of 16 hex chars + 1 NULL
	mono_byte public_key_token [MONO_PUBLIC_KEY_TOKEN_LENGTH];
	uint32_t hash_alg;
	uint32_t hash_len;
	uint32_t flags;
	uint16_t major, minor, build, revision, arch;
};

struct _MonoAssembly {
	/* 
	 * The number of appdomains which have this assembly loaded plus the number of 
	 * assemblies referencing this assembly through an entry in their image->references
	 * arrays. The later is needed because entries in the image->references array
	 * might point to assemblies which are only loaded in some appdomains, and without
	 * the additional reference, they can be freed at any time.
	 * The ref_count is initially 0.
	 */
	int ref_count; /* use atomic operations only */
	char *basedir;
	MonoAssemblyName aname;
	MonoImage *image;
	GSList *friend_assembly_names; /* Computed by mono_assembly_load_friends () */
	guint8 friend_assembly_names_inited;
	guint8 in_gac;
	guint8 dynamic;
	guint8 corlib_internal;
	gboolean ref_only;
	guint8 wrap_non_exception_throws;
	guint8 wrap_non_exception_throws_inited;
	guint8 jit_optimizer_disabled;
	guint8 jit_optimizer_disabled_inited;
	/* security manager flags (one bit is for lazy initialization) */
	guint32 ecma:2;		/* Has the ECMA key */
	guint32 aptc:2;		/* Has the [AllowPartiallyTrustedCallers] attributes */
	guint32 fulltrust:2;	/* Has FullTrust permission */
	guint32 unmanaged:2;	/* Has SecurityPermissionFlag.UnmanagedCode permission */
	guint32 skipverification:2;	/* Has SecurityPermissionFlag.SkipVerification permission */
};

typedef struct MonoVTable MonoVTable;
typedef struct _MonoThreadsSync MonoThreadsSync;

typedef struct {
	MonoVTable *vtable;
	MonoThreadsSync *synchronisation;
} MonoObject;

typedef struct _MonoClass MonoClass;
typedef struct _MonoMethodSignature MonoMethodSignature;

struct _MonoMethod {
	guint16 flags;  /* method flags */
	guint16 iflags; /* method implementation flags */
	guint32 token;
	MonoClass *klass;
	MonoMethodSignature *signature;
	/* name is useful mostly for debugging */
	const char *name;
	/* this is used by the inlining algorithm */
	unsigned int inline_info:1;
	unsigned int inline_failure:1;
	unsigned int wrapper_type:5;
	unsigned int string_ctor:1;
	unsigned int save_lmf:1;
	unsigned int dynamic:1; /* created & destroyed during runtime */
	unsigned int sre_method:1; /* created at runtime using Reflection.Emit */
	unsigned int is_generic:1; /* whenever this is a generic method definition */
	unsigned int is_inflated:1; /* whether we're a MonoMethodInflated */
	unsigned int skip_visibility:1; /* whenever to skip JIT visibility checks */
	unsigned int verification_success:1; /* whether this method has been verified successfully.*/
	/* TODO we MUST get rid of this field, it's an ugly hack nobody is proud of. */
	unsigned int is_mb_open : 1;		/* This is the fully open instantiation of a generic method_builder. Worse than is_tb_open, but it's temporary */
	signed int slot : 16;

	/*
	 * If is_generic is TRUE, the generic_container is stored in image->property_hash, 
	 * using the key MONO_METHOD_PROP_GENERIC_CONTAINER.
	 */
};

typedef struct _MonoMethod MonoMethod;

typedef MonoDomain* (*tmono_domain_get) (void);
typedef MonoAssembly* (*tmono_domain_assembly_open) (MonoDomain *domain, const char *name);
typedef MonoImage* (*tmono_assembly_get_image) (MonoAssembly* assembly);
typedef int (*tmono_jit_exec) (MonoDomain *domain, MonoAssembly *assembly, int argc, char *argv[]);
typedef MonoObject* (*tmono_runtime_invoke) (MonoMethod *method, void *obj, void **params, MonoObject **exc);
